#include "./type_check_visitor.h"
#include <unordered_map>
#include <map>
#include <iostream>

ClassEntity* TypeCheckVisitor::createGlobalClass(std::list<FunctionDeclaration*> functions, std::list<VariableDeclaration*>& variables) {
    // Add started scope
    scopesDeclarations.push_back(std::unordered_map<std::string, VariableEntity*>());
    auto packageClass = new ClassEntity();

    // Add builtIn functions
    for (auto & [identifier, functionSignature] : Semantic::BuiltInFunctions) {
        scopesDeclarations.back().emplace(identifier, new VariableEntity(functionSignature));
    }

    // Add package functions
    for (auto function : functions) {
        auto method =  new MethodEntity(function);
        if (!packageClass->addMethod(function->identifier, method)) {
            semantic->errors.push_back("Function " + function->identifier + " already declarated");
        } else {
            scopesDeclarations.back().emplace(function->identifier, new VariableEntity(method->toTypeEntity()));
        }
    }

    // Add package variables
    for (auto variableNode : variables) {
        variableNode->acceptVisitor(this);
        
        auto expressionsIter = variableNode->values.begin();
        for (auto identifier : variableNode->identifiersWithType->identifiers) {
            
            ExpressionAST* expressionNode = nullptr;
            if (expressionsIter != variableNode->values.end()) {
                expressionNode = (*expressionsIter);
                expressionsIter++;
            }

            auto field = new FieldEntity(scopesDeclarations[0][identifier]->type, expressionNode);

            if (!packageClass->addField(identifier, field)) {
                semantic->errors.push_back("Variable " + identifier + " already declarated");
            }
        }
    }

    for (auto & [identifier, methodSignature] : packageClass->getMethods()) {
        currentMethodEntity = methodSignature;

        scopesDeclarations.push_back(std::unordered_map<std::string, VariableEntity*>());

        // Get arguments from current MethodEntity
        for (auto & [id, type] : methodSignature->getArguments()) {
            scopesDeclarations.back().insert({id, new VariableEntity(type)});
            numberLocalVariables++;
        }

        lastAddedScopeInFuncDecl = true;
        methodSignature->getCodeBlock()->acceptVisitor(this);


        methodSignature->setNumberLocalVariables(numberLocalVariables);
        numberLocalVariables = 0;
    }

    return packageClass;
}


void TypeCheckVisitor::onStartVisit(BlockStatement* node) {
    if (!lastAddedScopeInFuncDecl) {
        scopesDeclarations.push_back(std::unordered_map<std::string, VariableEntity*>());
    }
    lastAddedScopeInFuncDecl = false;
}

void TypeCheckVisitor::onFinishVisit(BlockStatement* node) {
    for (auto & [id, var] : scopesDeclarations.back()) {
        if (var->numberUsage == 0) {
            semantic->errors.push_back("Unused variable " + id);
        }
    }

    scopesDeclarations.pop_back();
}

void TypeCheckVisitor::onFinishVisit(VariableDeclaration* node) {

    if (node->identifiersWithType->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->errors.push_back("Assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();
        for (auto id : node->identifiersWithType->identifiers) {
            numberLocalVariables++;

            if (scopesDeclarations.back().count(id)) {
                semantic->errors.push_back(id + " redeclared in this block");
                continue;
            }

            if (TypeEntity::IsBuiltInType(id)) {
                semantic->errors.push_back("Variable " + id + " collides with the 'builtin' type");
                continue;
            }
            
            if (node->identifiersWithType->type != nullptr) {
                auto generalType = new TypeEntity(node->identifiersWithType->type);

                // Compare types expressions with the declared type
                if (node->values.size() != 0 ) {
                    if (typesExpressions[(*currentValue)->nodeId]->equal(generalType)) {
                        scopesDeclarations.back()[id] = new VariableEntity(generalType, node->isConst);

                    } else {
                        semantic->errors.push_back("Assignment variable " + id + " must have equals types");
                    }
                } else {
                    scopesDeclarations.back()[id] = new VariableEntity(generalType, node->isConst);
                }
                
            } else {
                if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::Array && node->isConst) {
                    semantic->errors.push_back("Go does not support constant arrays, maps or slices");

                } else if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedFloat) {
                    scopesDeclarations.back()[id] = new VariableEntity(new TypeEntity(TypeEntity::Float), node->isConst);

                } else if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedInt) {
                    scopesDeclarations.back()[id] = new VariableEntity(new TypeEntity(TypeEntity::Int), node->isConst);

                } else {
                    scopesDeclarations.back()[id] = new VariableEntity(typesExpressions[(*currentValue)->nodeId], node->isConst);
                }
            }

            currentValue++;
        }
    }
}

void TypeCheckVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    
    if (node->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->errors.push_back("Short variable declaration: assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();

        for (auto id : node->identifiers) {
            numberLocalVariables++;

            if (TypeEntity::IsBuiltInType(id)) {
                semantic->errors.push_back("Variable " + id + " collides with the 'builtin' type");
            }

            if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedFloat) {
                scopesDeclarations.back()[id] = new VariableEntity(new TypeEntity(TypeEntity::Float));
                    
            } else if (typesExpressions[(*currentValue)->nodeId]->type == TypeEntity::UntypedInt) {
                scopesDeclarations.back()[id] = new VariableEntity(new TypeEntity(TypeEntity::Int));

            } else {
                scopesDeclarations.back()[id] = new VariableEntity(typesExpressions[(*currentValue)->nodeId]);
            }

            currentValue++;
        }
    }
}

void TypeCheckVisitor::onFinishVisit(IdentifierAsExpression* node) {

    // TODO create context class
    for (auto scope = scopesDeclarations.rbegin(); scope != scopesDeclarations.rend(); ++scope) {

        if ((*scope).count(node->identifier)) {
            typesExpressions[node->nodeId] = (*scope)[node->identifier]->type;
            (*scope)[node->identifier]->use();
            return ;
        }
    }
    
    semantic->errors.push_back("Undefined: " + node->identifier);
    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypeCheckVisitor::onFinishVisit(IntegerExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::UntypedInt);
}


void TypeCheckVisitor::onFinishVisit(BooleanExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Boolean);
}


void TypeCheckVisitor::onFinishVisit(FloatExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::UntypedFloat);
}


void TypeCheckVisitor::onFinishVisit(StringExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::String);
}


void TypeCheckVisitor::onFinishVisit(RuneExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Rune);
}


void TypeCheckVisitor::onFinishVisit(NilExpression* node) {
    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypeCheckVisitor::onFinishVisit(UnaryExpression* node) {
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


void TypeCheckVisitor::onFinishVisit(BinaryExpression* node) {
    if (typesExpressions[node->lhs->nodeId]->type == TypeEntity::Invalid) {
        typesExpressions[node->nodeId] = typesExpressions[node->lhs->nodeId];
        return ;

    } else if (typesExpressions[node->rhs->nodeId]->type == TypeEntity::Invalid) {
        typesExpressions[node->nodeId] = typesExpressions[node->rhs->nodeId];
        return ;
    }

    if (node->type == Addition || node->type == Subtraction || node->type == Multiplication || node->type == Division || node->type == Mod) {
        if (typesExpressions[node->lhs->nodeId]->isNumeric() && typesExpressions[node->rhs->nodeId]->isNumeric()) {
            typesExpressions[node->nodeId] = typesExpressions[node->lhs->nodeId]->determinePriorityType(typesExpressions[node->rhs->nodeId]);

        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have numeric expressions");
        }

    } else if (node->type == Or || node->type == And) {
        if (typesExpressions[node->lhs->nodeId]->type == TypeEntity::Boolean && typesExpressions[node->rhs->nodeId]->type == TypeEntity::Boolean) {
            typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Boolean);
        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have boolean expressions");
        }
    } else {
        if (typesExpressions[node->lhs->nodeId]->equal(typesExpressions[node->rhs->nodeId]) 
            && typesExpressions[node->lhs->nodeId]->type != TypeEntity::Array
            && typesExpressions[node->lhs->nodeId]->type != TypeEntity::UserType) {
            typesExpressions[node->nodeId] = new TypeEntity(TypeEntity::Boolean);
        } else {
            typesExpressions[node->nodeId] = new TypeEntity();
            semantic->errors.push_back(node->name() + " must have equals types expressions. Comparison of arrays and functions are'nt supported");
        }
    }
}


void TypeCheckVisitor::onFinishVisit(CallableExpression* node) {

    // Call declarated function
    TypeEntity* baseType = typesExpressions[node->base->nodeId];
    auto appendBase = dynamic_cast<IdentifierAsExpression*>(node->base);

    // TODO dirty hack
    if (appendBase != nullptr && appendBase->identifier == "append") {

        if (node->arguments.size() < 2) {
            semantic->errors.push_back("Append function must take more than two arguments");
            typesExpressions[node->nodeId] = new TypeEntity();
            return;
        }

        int index = 0;
        TypeEntity* arrayType = nullptr;
        for (auto arg : node->arguments) {
            if (index == 0) {
                if (typesExpressions[arg->nodeId]->type == TypeEntity::Array) {
                    arrayType = typesExpressions[arg->nodeId];
                } else {
                    semantic->errors.push_back("First agrument in 'append' must be array");
                    typesExpressions[node->nodeId] = new TypeEntity();
                    return;
                }
            } else {
                TypeEntity* elementType = std::get<ArraySignatureEntity*>(arrayType->value)->type;
                if (!elementType->equal(typesExpressions[arg->nodeId])) {
                    semantic->errors.push_back("Agrument " + std::to_string(index) + " in 'append' must has element array type");
                    typesExpressions[node->nodeId] = new TypeEntity();
                    return;
                }
            }
            index++;
        }
        
        typesExpressions[node->nodeId] = arrayType;
        return ;

    } else if (baseType->type == TypeEntity::Function) {

        std::list<ExpressionAST*>::const_iterator argExprType = node->arguments.begin();
        auto signature = std::get<FunctionSignatureEntity*>(baseType->value);

        int index = 0;
        for (auto argType : signature->argsTypes) {
            if (!argType->equal(typesExpressions[(*argExprType)->nodeId])) {
                typesExpressions[node->nodeId] = new TypeEntity();
                semantic->errors.push_back("Cannot use expression index " + std::to_string(index) + " in argument");
                return;
            }
            index++;
            argExprType++;
        }
        
        typesExpressions[node->nodeId] = signature->returnType;
        return ;

    } else if (baseType->type == TypeEntity::Invalid) {
        typesExpressions[node->nodeId] = new TypeEntity();
        return ;
    }

    semantic->errors.push_back("Cannot call non-function");
    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypeCheckVisitor::onFinishVisit(AccessExpression* node) {
    if (node->type == AccessExpressionEnum::Indexing) {
        if (typesExpressions[node->base->nodeId]->type != TypeEntity::Array) {
            semantic->errors.push_back("Base for indexing must be array");

        } else if (!typesExpressions[node->accessor->nodeId]->isInteger()) {
            semantic->errors.push_back("Index must be integer value");

        } else {
            typesExpressions[node->nodeId] = std::get<ArraySignatureEntity*>(typesExpressions[node->base->nodeId]->value)->type;
            return ;
        }
        typesExpressions[node->nodeId] = new TypeEntity();

    } else if (node->type == AccessExpressionEnum::FieldSelect && typesExpressions[node->accessor->nodeId]->type == TypeEntity::UserType) {
        // struct aren't supported yet
    }

    typesExpressions[node->nodeId] = new TypeEntity();
}


void TypeCheckVisitor::onFinishVisit(CompositeLiteral* node) {
    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type)) {
        auto declaredElementType = new TypeEntity(arrayType->arrayElementType);

        int index = 0;
        for (auto element : node->elements) {
            if (!typesExpressions[element->nodeId]->equal(declaredElementType)) {
                semantic->errors
                    .push_back("Array declarated type mismatch " + std::string("index ") + std::to_string(index));

                typesExpressions[node->nodeId] = new TypeEntity();
            }
            index++;
        }

        typesExpressions[node->nodeId] = new TypeEntity(arrayType);
    }
}


void TypeCheckVisitor::onFinishVisit(ElementCompositeLiteral* node) {

    if (std::holds_alternative<ExpressionAST *>(node->value)) {
        typesExpressions[node->nodeId] = typesExpressions[std::get<ExpressionAST*>(node->value)->nodeId];

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(node->value)) {
        TypeEntity* tentativeElementType = nullptr;

        for (auto el : std::get<std::list<ElementCompositeLiteral *>>(node->value)) {
            if (tentativeElementType == nullptr)
                tentativeElementType = typesExpressions[el->nodeId];
            else {
                tentativeElementType = tentativeElementType->determinePriorityType(typesExpressions[el->nodeId]);
            }
        }

        if (tentativeElementType->type == TypeEntity::Invalid) {
            typesExpressions[node->nodeId] = tentativeElementType;

        } else {
            typesExpressions[node->nodeId] = new TypeEntity(new ArraySignatureEntity(tentativeElementType));
        }
    }
}


void TypeCheckVisitor::onFinishVisit(AssignmentStatement* node) {
    
    // Check const variables
    for (auto var : node->lhs) {
        if (auto idVariable = dynamic_cast<IdentifierAsExpression*>(var)) {
                    
            VariableEntity* variable = nullptr;
            for (auto scope = scopesDeclarations.rbegin(); scope != scopesDeclarations.rend(); ++scope) {

                if ((*scope).count(idVariable->identifier)) {
                    variable = (*scope)[idVariable->identifier];

                    if (node->type == AssignmentEnum::SimpleAssign) {
                        (*scope)[idVariable->identifier]->numberUsage--;
                    }
                    
                    break;
                }
            }

            // TODO check right const expressions
            if (variable != nullptr && variable->isConst) {
                semantic->errors.push_back("Cannot assign to " + idVariable->identifier);
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

                if ((*indexIterator) == nullptr 
                        && !typesExpressions[(*idIterator)->nodeId]->equal(typesExpressions[(*valueIterator)->nodeId])) {
                    semantic->errors.push_back("Value by index " + std::to_string(index) + std::string(" cannot be represented"));

                } else if ((*indexIterator) != nullptr && typesExpressions[(*idIterator)->nodeId]->type == TypeEntity::Array) {
                    TypeEntity* typeElement = std::get<ArraySignatureEntity*>(typesExpressions[(*idIterator)->nodeId]->value)->type;

                    if (!typeElement->equal(typesExpressions[(*valueIterator)->nodeId])) {
                        semantic->errors.push_back("Value by index " + std::to_string(index) + std::string(" cannot be represented"));

                    } else if (!typesExpressions[(*indexIterator)->nodeId]->isInteger()) {
                        semantic->errors.push_back("Index must be integer value");
                    }
                }

                index++;
                indexIterator++;
                valueIterator++;
                idIterator++;
            }
        }

    } else {

        if (node->lhs.size() != 1) {
            semantic->errors.push_back("Unexpected " + node->name() + " expecting ':=', '=', or ','");

        } else if (node->lhs.size() != 1) {
            semantic->errors.push_back(
                "Assignment count mismatch " + std::to_string(node->lhs.size()) + " and " + std::to_string(node->rhs.size()));

        } else {
            auto index = node->indexes.begin();
            auto id = node->lhs.begin();
            auto value = node->rhs.begin();

            if ((*index) == nullptr && (!typesExpressions[(*value)->nodeId]->isNumeric() || !typesExpressions[(*id)->nodeId]->isNumeric())) {
                semantic->errors.push_back("Lhs and rhs in " + node->name() + " must be numeric");

            } else if ((*index) != nullptr && typesExpressions[(*id)->nodeId]->type == TypeEntity::Array) {
                TypeEntity* elementType = std::get<ArraySignatureEntity*>(typesExpressions[(*id)->nodeId]->value)->type;
                if (!elementType->isNumeric() || !typesExpressions[(*value)->nodeId]->isNumeric()) {
                    semantic->errors.push_back("Lhs and rhs in " + node->name() + " must be numeric");
                }
            }
        }
    }
}

void TypeCheckVisitor::onFinishVisit(ReturnStatement* node) {

    if (node->returnValues.size() > 1) {
        semantic->errors.push_back("'return' cannot take more than one value");

    } else if (currentMethodEntity->getReturnType()->type != TypeEntity::Void && node->returnValues.empty()) {
        semantic->errors.push_back("Missing return value");
    }

    for (auto value : node->returnValues) {
        if (!currentMethodEntity->getReturnType()->equal(typesExpressions[value->nodeId])) {
            semantic->errors.push_back("Cannot use this value for return");
        }
    }
}


void TypeCheckVisitor::onStartVisit(ExpressionStatement* node) {
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

void TypeCheckVisitor::onFinishVisit(WhileStatement* node) {
    if (typesExpressions[node->conditionExpression->nodeId]->type != TypeEntity::Boolean) {
        semantic->errors.push_back("The non-bool value used as a condition");
    }
}

void TypeCheckVisitor::onFinishVisit(IfStatement* node) {
    if (typesExpressions[node->condition->nodeId]->type != TypeEntity::Boolean) {
        semantic->errors.push_back("The non-bool value used as a condition");
    }
}
