#include "./types_visitor.h"
#include <unordered_map>
#include <map>
#include <iostream>

ClassEntity* TypesVisitor::createGlobalClass(std::list<FunctionDeclaration*> functions, std::list<VariableDeclaration*>& variables) {
    
    auto packageClass = new ClassEntity();

    // Add package functions
    for (auto function : functions) {
        auto method =  new MethodEntity(function);
        if (!packageClass->addMethod(function->identifier, method)) {
            semantic->errors.push_back(function->identifier + "redclared in block");
        } else {
            scopesDeclarations.add(function->identifier, new VariableEntity(method->toTypeEntity()));
        }
    }

    // Add package variables
    int indexBlankVariable = 0;
    for (auto variableNode : variables) {
        variableNode->acceptVisitor(this);
        
        auto expressionsIter = variableNode->values.begin();
        for (auto identifier : variableNode->identifiersWithType->identifiers) {
            
            ExpressionAST* expressionNode = nullptr;
            if (expressionsIter != variableNode->values.end()) {
                expressionNode = (*expressionsIter);
                expressionsIter++;
            }
            
            FieldEntity* field;

            // Blank static variables can't be deleted. We need to make them unique and unused
            if (identifier == "_") {
                identifier = "$_" + std::to_string(indexBlankVariable);
                indexBlankVariable++;
                field = new FieldEntity(new TypeEntity(TypeEntity::Any), expressionNode);
            } else {
                field = new FieldEntity(scopesDeclarations.find(identifier)->type, expressionNode);
            }
            
            if (!packageClass->addField(identifier, field)) {
                semantic->errors.push_back(identifier + " redclared in block");
            }
        }
    }

    // Analize methods
    for (auto & [identifier, methodSignature] : packageClass->getMethods()) {
        currentMethodEntity = methodSignature;

        scopesDeclarations.pushScope();

        // Get arguments from current MethodEntity
        for (auto & [id, type] : methodSignature->getArguments()) {
            scopesDeclarations.add(id, new VariableEntity(type, false, true));
            numberLocalVariables++;
        }

        lastAddedScopeInFuncDecl = true;
        methodSignature->getCodeBlock()->acceptVisitor(this);


        methodSignature->setNumberLocalVariables(numberLocalVariables);
        numberLocalVariables = 0;
    }

    return packageClass;
}


void TypesVisitor::onStartVisit(BlockStatement* node) {
    if (!lastAddedScopeInFuncDecl) {
        scopesDeclarations.pushScope();
    }
    lastAddedScopeInFuncDecl = false;
}

void TypesVisitor::onFinishVisit(BlockStatement* node) {
    for (auto & [id, var] : scopesDeclarations.getLastScope()) {

        if (var->numberUsage == 0 && !var->isArgument && !var->isConst) {
            semantic->errors.push_back("Unused variable " + id);
        }
    }

    scopesDeclarations.popScope();
}

void TypesVisitor::onFinishVisit(VariableDeclaration* node) {

    if (node->identifiersWithType->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->errors.push_back("Assignment count mismatch");

    } else {
        auto constCheckVisitor = ConstExpressionVisitor(this);

        auto currentValue = node->values.begin();
        for (auto id : node->identifiersWithType->identifiers) {
            if (id != "_") numberLocalVariables++;

            if (scopesDeclarations.findAtLastScope(id) != nullptr) {
                semantic->errors.push_back(id + " redeclared in this block");
                continue;
            }

            if (TypeEntity::IsBuiltInType(id)) {
                semantic->errors.push_back("Variable " + id + " collides with the 'builtin' type");
                continue;
            }

            // Const checking
            if (node->isConst && typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::Array) {
                semantic->errors.push_back("Go does not support constant arrays, maps or slices");

            } else if (node->isConst && !constCheckVisitor.isConstExpression(*currentValue)) {
                semantic->errors.push_back("Cannot assignment not const expression for " + id);
            }
            
            if (node->identifiersWithType->type != nullptr) {
                auto generalType = new TypeEntity(node->identifiersWithType->type);

                // Compare types expressions with the declared type
                if (node->values.size() != 0 ) {
                    if (typesExpressions[(*currentValue)->nodeId]->equal(generalType)) {
                        typesExpressions[(*currentValue)->nodeId] = typesExpressions[(*currentValue)->nodeId]->determinePriorityType(generalType);
                        scopesDeclarations.add(id, new VariableEntity(generalType, node->isConst));

                    } else {
                        semantic->errors.push_back("Assignment variable " + id + " must have equals types");
                    }
                } else {
                    scopesDeclarations.add(id, new VariableEntity(generalType, node->isConst));
                }
                
            } else {
                if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedFloat) {
                    scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Float), node->isConst));

                } else if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedInt) {
                    scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Int), node->isConst));

                } else {
                    scopesDeclarations.add(id, new VariableEntity(typesExpressions[(*currentValue)->nodeId], node->isConst));
                }
            }

            currentValue++;
        }
    }
}

void TypesVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    
    if (node->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->errors.push_back("Short variable declaration: assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();

        for (auto id : node->identifiers) {
            if (id != "_") numberLocalVariables++;

            if (TypeEntity::IsBuiltInType(id)) {
                semantic->errors.push_back("Variable " + id + " collides with the 'builtin' type");
            }

            if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedFloat) {
                scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Float)));
                    
            } else if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedInt) {
                scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Int)));

            } else {
                scopesDeclarations.add(id, new VariableEntity(typesExpressions[(*currentValue)->nodeId]));
            }

            currentValue++;
        }
    }
}

void TypesVisitor::onFinishVisit(IdentifierAsExpression* node) {

    VariableEntity* variable = scopesDeclarations.find(node->identifier);

    if (variable != nullptr) {
        typesExpressions[node->nodeId] = variable->type;
        if (!node->isDestination) variable->use();
        return ;

    } else if (Semantic::IsBuiltInFunction(node->identifier)) {
        typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::BuiltInFunction, node->identifier);
        return;

    } else if (node->identifier == "_" && node->isDestination) {
        typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Any);
        return;
    }
    
    std::string errorMessage = "Undefined: " + node->identifier;

    if (node->identifier == "_" && !node->isDestination) {
        errorMessage = "Cannot use _ as value";
    }

    semantic->errors.push_back(errorMessage);
    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypesVisitor::onFinishVisit(IntegerExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::UntypedInt);
}


void TypesVisitor::onFinishVisit(BooleanExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Boolean);
}


void TypesVisitor::onFinishVisit(FloatExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::UntypedFloat);
}


void TypesVisitor::onFinishVisit(StringExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::String);
}


void TypesVisitor::onFinishVisit(NilExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypesVisitor::onFinishVisit(UnaryExpression* node) {
    if (typesExpressions[node->expression->nodeId]->type == TypeEntity::Invalid) {
        typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        return ;
    }

    if (node->type == UnaryExpressionEnum::UnaryNot) {
        if (typesExpressions[node->expression->nodeId]->type == TypeEntity::Boolean) {
            typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have boolean expression");
        }

    } else if (node->type == UnaryExpressionEnum::Variadic) {
        if (typesExpressions[node->expression->nodeId]->type == TypeEntity::Array) {
            typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have array expression");
        }

    } else {
        if (typesExpressions[node->expression->nodeId]->isNumeric()) {
            typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have numeric expression");
        }
    }
}


void TypesVisitor::onFinishVisit(BinaryExpression* node) {
    auto leftExprType = typesExpressions[node->lhs->nodeId];
    auto rightExprType = typesExpressions[node->rhs->nodeId];

    if (leftExprType->type == TypeEntity::Invalid) {
        typesExpressions[node->nodeId] = leftExprType;
        return ;

    } else if (rightExprType->type == TypeEntity::Invalid) {
        typesExpressions[node->nodeId] = rightExprType;
        return ;
    }

    if (leftExprType->type == TypeEntity::String && rightExprType->type == TypeEntity::String && node->type == Addition) {
        typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::String);

    } else if (node->type == Addition || node->type == Subtraction || node->type == Multiplication || node->type == Division || node->type == Mod) {

        if (leftExprType->isNumeric() && rightExprType->isNumeric() && leftExprType->equal(rightExprType)) {
            auto resultTypeExression = leftExprType->determinePriorityType(rightExprType);

            typesExpressions[node->nodeId] = resultTypeExression;
            typesExpressions[node->lhs->nodeId] = resultTypeExression;
            typesExpressions[node->rhs->nodeId] = resultTypeExression;

        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have same numeric types expressions");
        }

    } else if (node->type == Or || node->type == And) {
        if (leftExprType->type == TypeEntity::Boolean && rightExprType->type == TypeEntity::Boolean) {
            typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Boolean);
        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have boolean expressions");
        }
    } else {
        if (leftExprType->equal(rightExprType) && (leftExprType->isFloat()
            || leftExprType->isInteger()
            || leftExprType->type == TypeEntity::String
            || ((leftExprType->type == TypeEntity::Boolean || leftExprType->type == TypeEntity::Array) 
            && (node->type == BinaryExpressionEnum::Equal || node->type == BinaryExpressionEnum::NotEqual)))
        ) {
            typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Boolean);

        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have equal types of expressions. Comparison of booleans, arrays and functions are'nt supported");
        }
    }
}


void TypesVisitor::onFinishVisit(CallableExpression* node) {

    // Call declarated function
    TypeEntity* baseType = typesExpressions[node->base->nodeId];

    if (baseType->type == TypeEntity::Function) {

        std::list<ExpressionAST*>::const_iterator argExprType = node->arguments.begin();
        auto signature = std::get<FunctionSignatureEntity*>(baseType->value);

        if (signature->argsTypes.size() != node->arguments.size()) {
            semantic->errors.push_back("Invalid number of arguments");
            typesExpressions[node->nodeId] = new TypeEntity();
            return;
        }

        int index = 0;
        for (auto argType : signature->argsTypes) {
            if (!argType->equal(typesExpressions[(*argExprType)->nodeId])) {
                typesExpressions[node->nodeId] = new TypeEntity();
                semantic->errors.push_back("Cannot use expression index " + std::to_string(index) + " in argument");
                return;

            } else {
                typesExpressions[(*argExprType)->nodeId] = typesExpressions[(*argExprType)->nodeId]->determinePriorityType(argType);
            }
            index++;
            argExprType++;
        }
        
        typesExpressions[node->nodeId] = signature->returnType;
        return ;
    
    } else if (baseType->type == TypeEntity::BuiltInFunction) {
        bool isCheckBuiltInFunctionSuccess = defineTypeBuiltInFunction(node);

        if (!isCheckBuiltInFunctionSuccess) {
            typesExpressions[node->nodeId] = new TypeEntity();
        }
        return ;
    }

    semantic->errors.push_back("Cannot call non-function");
    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypesVisitor::onFinishVisit(AccessExpression* node) {
    if (node->type == AccessExpressionEnum::Indexing) {
        if (typesExpressions[node->base->nodeId]->type != TypeEntity::Array) {
            semantic->errors.push_back("Base for indexing must be array");

        } else if (!typesExpressions[node->accessor->nodeId]->isInteger()) {
            semantic->errors.push_back("Index must be integer value");

        } else {
            typesExpressions[node->nodeId] = std::get<ArraySignatureEntity*>(typesExpressions[node->base->nodeId]->value)->elementType;
            return ;
        }
        typesExpressions[node->nodeId] = new TypeEntity();

    } else if (node->type == AccessExpressionEnum::FieldSelect || typesExpressions[node->accessor->nodeId]->type == TypeEntity::UserType) {
        // struct aren't supported yet
    }

    typesExpressions[node->nodeId] = new TypeEntity();
}

void TypesVisitor::onStartVisit(CompositeLiteral* node) {

    // Если это массив, нам нужно записать его тип и запомнить id узла для проверки элементов
    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type)) {
        typesExpressions[node->nodeId] = new TypeEntity(arrayType);
        nodeIdCurrentArray = node->nodeId;
        indexCurrentAxisArray = 0;
    }
}

void TypesVisitor::onStartVisit(ElementCompositeLiteral* node) {
    indexCurrentAxisArray++;
}


void TypesVisitor::onFinishVisit(CompositeLiteral* node) {

    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type)) {

        if (arrayType->dimensions < node->elements.size()) {
            semantic->errors.push_back("Array has more elements than declarated");
            typesExpressions[node->nodeId] = new TypeEntity();
            return;
        }

        // Тип текущего массива был вычислен при первом заходе в этот узел
        auto declaredElementType = std::get<ArraySignatureEntity*>(typesExpressions[node->nodeId]->value)->elementType;

        int index = 0;
        for (auto element : node->elements) {
            if (!typesExpressions[element->nodeId]->equal(declaredElementType)) {
                semantic->errors
                    .push_back("Array declarated type mismatch " + std::string("index ") + std::to_string(index));

                typesExpressions[node->nodeId] = new TypeEntity();
            }
            index++;
        }

        nodeIdCurrentArray = -1;
        indexCurrentAxisArray = -1;
    }
}


void TypesVisitor::onFinishVisit(ElementCompositeLiteral* node) {

    TypeEntity* declaratedElementType = 
        std::get<ArraySignatureEntity*>(typesExpressions[nodeIdCurrentArray]->value)->typeAxis(indexCurrentAxisArray);

    if (std::holds_alternative<ExpressionAST *>(node->value)) {
        auto expression = std::get<ExpressionAST*>(node->value);

        if (declaratedElementType->equal(typesExpressions[expression->nodeId])) {
            typesExpressions[expression->nodeId] = declaratedElementType->determinePriorityType(typesExpressions[expression->nodeId]);
            typesExpressions[node->nodeId] = declaratedElementType;

        } else {
            semantic->errors.push_back("Expression at " + std::to_string(indexCurrentAxisArray) + " axis has invalid type");
            typesExpressions[node->nodeId] = new TypeEntity();
        }

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(node->value)) {

        if (auto declaratedTypeAxis = std::get<ArraySignatureEntity*>(declaratedElementType->value)) {

            if (declaratedTypeAxis->dims < std::get<std::list<ElementCompositeLiteral*>>(node->value).size() ) {
                semantic->errors.push_back("Array at " + std::to_string(indexCurrentAxisArray) + " axis has many values");
                typesExpressions[node->nodeId] = new TypeEntity();

            } else {
                typesExpressions[node->nodeId] = declaratedElementType;
            }

        } else {
            semantic->errors.push_back("Expression at " + std::to_string(indexCurrentAxisArray) + " axis has invalid type");
            typesExpressions[node->nodeId] = new TypeEntity();
        }
    }

    indexCurrentAxisArray--;
}


void TypesVisitor::onFinishVisit(AssignmentStatement* node) {
    
    // Check const variables
    for (auto var : node->lhs) {
        if (auto idVariable = dynamic_cast<IdentifierAsExpression*>(var)) {
                    
            VariableEntity* variable = scopesDeclarations.find(idVariable->identifier);

            if (variable != nullptr && variable->isConst) {
                semantic->errors.push_back("Cannot assign to const " + idVariable->identifier);
            }
            
        } else if (dynamic_cast<AccessExpression*>(var) == nullptr) {
            semantic->errors.push_back("Cannot assign to " + var->name());
        }
    }

    if (node->type == AssignmentEnum::SimpleAssign) {

        if (node->lhs.size() != node->rhs.size()) {
            semantic->errors.push_back(
                "Assignment count mismatch " + std::to_string(node->lhs.size()) + " and " + std::to_string(node->rhs.size()));

        } else {
            
            int index = 0;
            ExpressionList::iterator indexIterator = node->indexes.begin();
            ExpressionList::iterator valueIterator = node->rhs.begin();
            ExpressionList::iterator idIterator = node->lhs.begin();

            while (indexIterator != node->indexes.end() && idIterator != node->lhs.end() && valueIterator != node->rhs.end()) {

                // if right and left parts have same types ()
                if ((*indexIterator) == nullptr) {

                    if (!typesExpressions[(*idIterator)->nodeId]->equal(typesExpressions[(*valueIterator)->nodeId])) {
                        semantic->errors.push_back("Value by index " + std::to_string(index) + std::string(" cannot be represented for assignment"));

                    } else {
                        typesExpressions[(*valueIterator)->nodeId] = 
                            typesExpressions[(*valueIterator)->nodeId]->determinePriorityType(typesExpressions[(*idIterator)->nodeId]);
                    }

                } else {

                    // Indexing access expression must have the base as array type
                    if (typesExpressions[(*idIterator)->nodeId]->type == TypeEntity::Array) {

                        TypeEntity* typeElement = std::get<ArraySignatureEntity*>(typesExpressions[(*idIterator)->nodeId]->value)->elementType;

                        if (!typeElement->equal(typesExpressions[(*valueIterator)->nodeId])) {
                            semantic->errors.push_back("Value by index " + std::to_string(index) + std::string(" cannot be represented for assignment"));

                        } else if (!typesExpressions[(*indexIterator)->nodeId]->isInteger()) {
                            semantic->errors.push_back("Index must be integer value");

                        } else {
                            typesExpressions[(*valueIterator)->nodeId] = typesExpressions[(*valueIterator)->nodeId]->determinePriorityType(typeElement);
                        }
                    } else {
                        semantic->errors.push_back("Cannot get value by index. Not array");
                    }
                }

                index++;
                indexIterator++;
                valueIterator++;
                idIterator++;
            }
        }
    }
}

void TypesVisitor::onFinishVisit(ReturnStatement* node) {

    if (node->returnValues.size() > 1) {
        semantic->errors.push_back("Return cannot take more than one value");

    } else if (currentMethodEntity->getReturnType()->type != TypeEntity::Void && node->returnValues.empty()) {
        semantic->errors.push_back("Missing return value");
    }

    for (auto value : node->returnValues) {
        if (!currentMethodEntity->getReturnType()->equal(typesExpressions[value->nodeId])) {
            semantic->errors.push_back("Cannot use this value for return");
        }
    }
}


void TypesVisitor::onStartVisit(ExpressionStatement* node) {
    // Increment and decrement can be statements
    if (auto unaryExpression = dynamic_cast<UnaryExpression*>(node->expression)) {
        if (unaryExpression->type == UnaryExpressionEnum::Decrement || unaryExpression->type == UnaryExpressionEnum::Increment) {
            return;
        }
        
    } else if (auto functionCall = dynamic_cast<CallableExpression*>(node->expression)) {
        // With the exception of specific built-in functions and conversions, callable expressions can appear in statement context.
        if ( auto identifiedBase = dynamic_cast<IdentifierAsExpression*>(functionCall->base)) {
            if (identifiedBase->identifier != "append" && identifiedBase->identifier != "len" && !TypeEntity::IsBuiltInType(identifiedBase->identifier))
            return;
        }
    }
    
    semantic->errors.push_back(node->expression->name() + " expression not available for statement");
}

void TypesVisitor::onFinishVisit(WhileStatement* node) {
    if (typesExpressions[node->conditionExpression->nodeId]->type != TypeEntity::Boolean) {
        semantic->errors.push_back("The non-bool value used as a condition in loop");
    }
}

void TypesVisitor::onFinishVisit(IfStatement* node) {
    if (typesExpressions[node->condition->nodeId]->type != TypeEntity::Boolean) {
        semantic->errors.push_back("The non-bool value used as a condition in if statement");
    }
}

void TypesVisitor::onFinishVisit(SwitchStatement* node) {
    TypeEntity* typeSwitchExpression = typesExpressions[node->expression->nodeId];

    int index = 0;
    for (auto caseClause : node->clauseList) {

        if (caseClause->expressionCase) {
            TypeEntity* caseExpressionType = typesExpressions[caseClause->expressionCase->nodeId];

            if (caseExpressionType->equal(typeSwitchExpression)) {
                typesExpressions[caseClause->expressionCase->nodeId] = typeSwitchExpression;
                
            } else {
                semantic->errors.push_back("The type of expression in case " + std::to_string(index) + " statement should be the same as in switch");
            }
        }

        index++;
    }
}

std::unordered_map<size_t, TypeEntity*> TypesVisitor::getTypesExpressions() const {
    return this->typesExpressions;
}


bool ConstExpressionVisitor::isConstExpression(ExpressionAST* expr) {
    constValid = true;
    expr->acceptVisitor(this);
    return constValid;
}

void ConstExpressionVisitor::onFinishVisit(IdentifierAsExpression* node) {
    VariableEntity* variable = typesVisitor->scopesDeclarations.find(node->identifier);

    if (variable != nullptr) {
        constValid &= variable->isConst;
    }
}

void ConstExpressionVisitor::onFinishVisit(CallableExpression* node) {
    constValid &= false;
}

void ConstExpressionVisitor::onFinishVisit(AccessExpression* node) {
    constValid &= false;
}

void ConstExpressionVisitor::onFinishVisit(CompositeLiteral* node) {
    constValid &= false;
}