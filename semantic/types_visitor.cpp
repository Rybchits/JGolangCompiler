#include "./types_visitor.h"

#include <unordered_map>
#include <map>
#include <iostream>

void TypesVisitor::analyzePackageClass(ClassEntity* packageClass, std::vector<std::string>& idsConstants) {
    
    // Added signatures functions in scope
    for (auto & [methodName, methodSignature] : packageClass->getMethods()) {
        scopesDeclarations.add(methodName, new VariableEntity(methodSignature->toTypeEntity()));
    }

    // Analize package variables
    // TODO Variables are initialized in order of declarations. Differs from source language
    for (auto & [variableName, variableEntity] : packageClass->getFields()) {
        
        if (variableEntity->declaration != nullptr) {
            variableEntity->declaration->acceptVisitor(this);
            auto typeExpression = variableEntity->declaration->typeExpression;

            bool isConstVariable = std::find(idsConstants.begin(), idsConstants.end(), variableName) != idsConstants.end();

            if (variableEntity->type->type != TypeEntity::Any 
                && variableEntity->type->type != TypeEntity::Invalid
                && !typeExpression->equal(variableEntity->type)
            ) {
                semantic->addError("Cannot initialize " + variableName);
                
            } else if (isConstVariable && !constCheckVisitor.isConstExpression(variableEntity->declaration)) {
                semantic->addError("Expression is not constant");

            } else {
                scopesDeclarations.add(variableName, new VariableEntity(typeExpression, isConstVariable));
                variableEntity->type = typeExpression;
            }
        }
    }

    // Analize methods
    for (auto & [methodName, methodSignature] : packageClass->getMethods()) {
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
            semantic->addError("Unused variable " + id);
        }
    }

    scopesDeclarations.popScope();
}

void TypesVisitor::onFinishVisit(VariableDeclaration* node) {

    if (node->identifiersWithType->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->addError("Assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();
        for (auto id : node->identifiersWithType->identifiers) {
            if (id != "_") numberLocalVariables++;

            if (scopesDeclarations.findAtLastScope(id) != nullptr) {
                semantic->addError(id + " redeclared in this block");
                continue;
            }

            if (TypeEntity::IsBuiltInType(id)) {
                semantic->addError("Variable " + id + " collides with the 'builtin' type");
                continue;
            }

            // Const checking
            if (node->isConst && (*currentValue)->typeExpression->type == TypeEntity::Array) {
                semantic->addError("Go does not support constant arrays, maps or slices");

            } else if (node->isConst && !constCheckVisitor.isConstExpression(*currentValue)) {
                semantic->addError("Cannot assignment not const expression for " + id);
            }
            
            if (node->identifiersWithType->type != nullptr) {
                auto generalType = new TypeEntity(node->identifiersWithType->type);

                // Compare types expressions with the declared type
                if (node->values.size() != 0 ) {
                    if ((*currentValue)->typeExpression->equal(generalType)) {
                        (*currentValue)->typeExpression = (*currentValue)->typeExpression->determinePriorityType(generalType);
                        scopesDeclarations.add(id, new VariableEntity(generalType, node->isConst));

                    } else {
                        semantic->addError("Assignment variable " + id + " must have equals types");
                    }
                } else {
                    scopesDeclarations.add(id, new VariableEntity(generalType, node->isConst));
                }
                
            } else {
                if ((*currentValue)->typeExpression->type == TypeEntity::UntypedFloat) {
                    scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Float), node->isConst));

                } else if ((*currentValue)->typeExpression->type == TypeEntity::UntypedInt) {
                    scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Int), node->isConst));

                } else {
                    scopesDeclarations.add(id, new VariableEntity((*currentValue)->typeExpression, node->isConst));
                }
            }

            currentValue++;
        }
    }
}

void TypesVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    
    if (node->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->addError("Short variable declaration: assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();

        for (auto id : node->identifiers) {
            if (id != "_") numberLocalVariables++;

            if (TypeEntity::IsBuiltInType(id)) {
                semantic->addError("Variable " + id + " collides with the 'builtin' type");
            }

            if ((*currentValue)->typeExpression->type == TypeEntity::UntypedFloat) {
                scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Float)));
                    
            } else if ((*currentValue)->typeExpression->type == TypeEntity::UntypedInt) {
                scopesDeclarations.add(id, new VariableEntity(new TypeEntity(TypeEntity::Int)));

            } else {
                scopesDeclarations.add(id, new VariableEntity((*currentValue)->typeExpression));
            }

            currentValue++;
        }
    }
}

void TypesVisitor::onFinishVisit(IdentifierAsExpression* node) {

    VariableEntity* variable = scopesDeclarations.find(node->identifier);

    if (variable != nullptr) {
        node->typeExpression = variable->type;
        if (!node->isDestination) variable->use();
        return ;

    } else if (Semantic::IsBuiltInFunction(node->identifier)) {
        node->typeExpression = new TypeEntity(TypeEntity::BuiltInFunction, node->identifier);
        return;

    } else if (node->identifier == "_" && node->isDestination) {
        node->typeExpression = new TypeEntity(TypeEntity::Any);
        return;
    }
    
    std::string errorMessage = "Undefined: " + node->identifier;

    if (node->identifier == "_" && !node->isDestination) {
        errorMessage = "Cannot use _ as value";
    }

    semantic->addError(errorMessage);
    node->typeExpression = new TypeEntity();
}


void TypesVisitor::onFinishVisit(IntegerExpression* node) {
    node->typeExpression = new TypeEntity(TypeEntity::UntypedInt);
}


void TypesVisitor::onFinishVisit(BooleanExpression* node) {
    node->typeExpression = new TypeEntity(TypeEntity::Boolean);
}


void TypesVisitor::onFinishVisit(FloatExpression* node) {
    node->typeExpression = new TypeEntity(TypeEntity::UntypedFloat);
}


void TypesVisitor::onFinishVisit(StringExpression* node) {
    node->typeExpression = new TypeEntity(TypeEntity::String);
}


void TypesVisitor::onFinishVisit(NilExpression* node) {
    node->typeExpression = new TypeEntity();
}


void TypesVisitor::onFinishVisit(UnaryExpression* node) {
    if (node->expression->typeExpression->type == TypeEntity::Invalid) {
        node->typeExpression = node->expression->typeExpression;
        return ;
    }

    if (node->type == UnaryExpression::UnaryNot) {
        if (node->expression->typeExpression->type == TypeEntity::Boolean) {
            node->typeExpression = node->expression->typeExpression;
        } else {
            node->typeExpression = new TypeEntity();
            semantic->addError(node->name() + " must have boolean expression");
        }

    } else if (node->type == UnaryExpression::Variadic) {
        if (node->expression->typeExpression->type == TypeEntity::Array) {
            node->typeExpression = node->expression->typeExpression;
        } else {
            node->typeExpression = new TypeEntity();
            semantic->addError(node->name() + " must have array expression");
        }

    } else {
        if (node->expression->typeExpression->isNumeric()) {
            node->typeExpression = node->expression->typeExpression;
        } else {
            node->typeExpression = new TypeEntity();
            semantic->addError(node->name() + " must have numeric expression");
        }
    }
}


void TypesVisitor::onFinishVisit(BinaryExpression* node) {
    auto leftExprType = node->lhs->typeExpression;
    auto rightExprType = node->rhs->typeExpression;

    if (leftExprType->type == TypeEntity::Invalid) {
        node->typeExpression = leftExprType;
        return ;

    } else if (rightExprType->type == TypeEntity::Invalid) {
        node->typeExpression = rightExprType;
        return ;
    }

    if (leftExprType->type == TypeEntity::String && rightExprType->type == TypeEntity::String && node->type == BinaryExpression::Addition) {
        node->typeExpression = new TypeEntity(TypeEntity::String);

    } else if (node->type == BinaryExpression::Addition || node->type == BinaryExpression::Subtraction 
    || node->type == BinaryExpression::Multiplication || node->type == BinaryExpression::Division || node->type == BinaryExpression::Mod) {

        if (leftExprType->isNumeric() && rightExprType->isNumeric() && leftExprType->equal(rightExprType)) {
            auto resultTypeExression = leftExprType->determinePriorityType(rightExprType);

            node->typeExpression = resultTypeExression;
            node->lhs->typeExpression = resultTypeExression;
            node->rhs->typeExpression = resultTypeExression;

        } else {
            node->typeExpression = new TypeEntity();
            semantic->addError(node->name() + " must have same numeric types expressions");
        }

    } else if (node->type == BinaryExpression::Or || node->type == BinaryExpression::And) {
        if (leftExprType->type == TypeEntity::Boolean && rightExprType->type == TypeEntity::Boolean) {
            node->typeExpression = new TypeEntity(TypeEntity::Boolean);
        } else {
            node->typeExpression = new TypeEntity();
            semantic->addError(node->name() + " must have boolean expressions");
        }
    } else {
        if (leftExprType->equal(rightExprType) && (leftExprType->isFloat()
            || leftExprType->isInteger()
            || leftExprType->type == TypeEntity::String
            || ((leftExprType->type == TypeEntity::Boolean || leftExprType->type == TypeEntity::Array) 
            && (node->type == BinaryExpression::Equal || node->type == BinaryExpression::NotEqual)))
        ) {
            node->typeExpression = new TypeEntity(TypeEntity::Boolean);

        } else {
            node->typeExpression = new TypeEntity();
            semantic->addError(node->name() + " must have equal types of expressions. Comparison of booleans, arrays and functions are'nt supported");
        }
    }
}


void TypesVisitor::onFinishVisit(CallableExpression* node) {

    // Call declarated function
    TypeEntity* baseType = node->base->typeExpression;

    if (baseType->type == TypeEntity::Function) {

        std::list<ExpressionAST*>::const_iterator argExprType = node->arguments.begin();
        auto signature = std::get<FunctionSignatureEntity*>(baseType->value);

        if (signature->argsTypes.size() != node->arguments.size()) {
            semantic->addError("Invalid number of arguments");
            node->typeExpression = new TypeEntity();
            return;
        }

        int index = 0;
        for (auto argType : signature->argsTypes) {
            if (!argType->equal((*argExprType)->typeExpression)) {
                node->typeExpression = new TypeEntity();
                semantic->addError("Cannot use expression index " + std::to_string(index) + " in argument");
                return;

            } else {
                (*argExprType)->typeExpression = (*argExprType)->typeExpression->determinePriorityType(argType);
            }
            index++;
            argExprType++;
        }
        
        node->typeExpression = signature->returnType;
        return ;
    
    } else if (baseType->type == TypeEntity::BuiltInFunction) {
        bool isCheckBuiltInFunctionSuccess = defineTypeBuiltInFunction(node);

        if (!isCheckBuiltInFunctionSuccess) {
            node->typeExpression = new TypeEntity();
        }
        return ;
    }

    semantic->addError("Cannot call non-function");
    node->typeExpression = new TypeEntity();
}


void TypesVisitor::onFinishVisit(AccessExpression* node) {
    switch (node->type)
    {
    case AccessExpression::Indexing:
        if (node->base->typeExpression->type != TypeEntity::Array) {
            semantic->addError("Base for indexing must be array");

        } else if (!node->accessor->typeExpression->isInteger()) {
            semantic->addError("Index must be integer value");

        } else {
            node->typeExpression = std::get<ArraySignatureEntity*>(node->base->typeExpression->value)->elementType;
            return ;
        }
        node->typeExpression = new TypeEntity();
        break;

    case AccessExpression::FieldSelect:
        break;
    
    default:
        break;
    }

    node->typeExpression = new TypeEntity();
}

void TypesVisitor::onStartVisit(CompositeLiteral* node) {

    // Если это массив, нам нужно записать его тип и запомнить id узла для проверки элементов
    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type)) {
        node->typeExpression = new TypeEntity(arrayType);
        typeCurrentArray = node->typeExpression;
        indexCurrentAxisArray = 0;
    }
}

void TypesVisitor::onStartVisit(ElementCompositeLiteral* node) {
    indexCurrentAxisArray++;
}


void TypesVisitor::onFinishVisit(CompositeLiteral* node) {

    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type)) {

        if (arrayType->dimensions < node->elements.size()) {
            semantic->addError("Array has more elements than declarated");
            node->typeExpression = new TypeEntity();
            return;
        }

        // Тип текущего массива был вычислен при первом заходе в этот узел
        auto declaredElementType = std::get<ArraySignatureEntity*>(node->typeExpression->value)->elementType;

        int index = 0;
        for (auto element : node->elements) {
            if (!element->typeExpression->equal(declaredElementType)) {
                semantic->addError("Array declarated type mismatch " + std::string("index ") + std::to_string(index));

                node->typeExpression = new TypeEntity();
            }
            index++;
        }

        typeCurrentArray = nullptr;
        indexCurrentAxisArray = -1;
    }
}


void TypesVisitor::onFinishVisit(ElementCompositeLiteral* node) {

    TypeEntity* declaratedElementType = 
        std::get<ArraySignatureEntity*>(typeCurrentArray->value)->typeAxis(indexCurrentAxisArray);

    if (std::holds_alternative<ExpressionAST *>(node->value)) {
        auto expression = std::get<ExpressionAST*>(node->value);

        if (declaratedElementType->equal(expression->typeExpression)) {
            expression->typeExpression = declaratedElementType->determinePriorityType(expression->typeExpression);
            node->typeExpression = declaratedElementType;

        } else {
            semantic->addError("Expression at " + std::to_string(indexCurrentAxisArray) + " axis has invalid type");
            node->typeExpression = new TypeEntity();
        }

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(node->value)) {

        if (auto declaratedTypeAxis = std::get<ArraySignatureEntity*>(declaratedElementType->value)) {

            if (declaratedTypeAxis->dims < std::get<std::list<ElementCompositeLiteral*>>(node->value).size() ) {
                semantic->addError("Array at " + std::to_string(indexCurrentAxisArray) + " axis has many values");
                node->typeExpression = new TypeEntity();

            } else {
                node->typeExpression = declaratedElementType;
            }

        } else {
            semantic->addError("Expression at " + std::to_string(indexCurrentAxisArray) + " axis has invalid type");
            node->typeExpression = new TypeEntity();
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
                semantic->addError("Cannot assign to const " + idVariable->identifier);
            }
            
        } else if (dynamic_cast<AccessExpression*>(var) == nullptr) {
            semantic->addError("Cannot assign to " + var->name());
        }
    }

    if (node->type == AssignmentStatement::SimpleAssign) {

        if (node->lhs.size() != node->rhs.size()) {
            semantic->addError(
                "Assignment count mismatch " + std::to_string(node->lhs.size()) + " and " + std::to_string(node->rhs.size()));

        } else {
            
            int index = 0;
            ExpressionList::iterator indexIterator = node->indexes.begin();
            ExpressionList::iterator valueIterator = node->rhs.begin();
            ExpressionList::iterator idIterator = node->lhs.begin();

            while (indexIterator != node->indexes.end() && idIterator != node->lhs.end() && valueIterator != node->rhs.end()) {

                // if right and left parts have same types ()
                if ((*indexIterator) == nullptr) {

                    if (!(*idIterator)->typeExpression->equal((*valueIterator)->typeExpression)) {
                        semantic->addError("Value by index " + std::to_string(index) + std::string(" cannot be represented for assignment"));

                    } else {
                        (*valueIterator)->typeExpression = (*valueIterator)->typeExpression->determinePriorityType((*idIterator)->typeExpression);
                    }

                } else {

                    // Indexing access expression must have the base as array type
                    if ((*idIterator)->typeExpression->type == TypeEntity::Array) {

                        TypeEntity* typeElement = std::get<ArraySignatureEntity*>((*idIterator)->typeExpression->value)->elementType;

                        if (!typeElement->equal((*valueIterator)->typeExpression)) {
                            semantic->addError("Value by index " + std::to_string(index) + std::string(" cannot be represented for assignment"));

                        } else if (!(*indexIterator)->typeExpression->isInteger()) {
                            semantic->addError("Index must be integer value");

                        } else {
                            (*valueIterator)->typeExpression = (*valueIterator)->typeExpression->determinePriorityType(typeElement);
                        }
                    } else {
                        semantic->addError("Cannot get value by index. Not array");
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
        semantic->addError("Return cannot take more than one value");

    } else if (currentMethodEntity->getReturnType()->type != TypeEntity::Void && node->returnValues.empty()) {
        semantic->addError("Missing return value");
    }

    for (auto value : node->returnValues) {
        if (!currentMethodEntity->getReturnType()->equal(value->typeExpression)) {
            semantic->addError("Cannot use this value for return");
        }
    }
}


void TypesVisitor::onStartVisit(ExpressionStatement* node) {
    // Increment and decrement can be statements
    if (auto unaryExpression = dynamic_cast<UnaryExpression*>(node->expression)) {
        if (unaryExpression->type == UnaryExpression::Decrement || unaryExpression->type == UnaryExpression::Increment) {
            return;
        }
        
    } else if (auto functionCall = dynamic_cast<CallableExpression*>(node->expression)) {
        // With the exception of specific built-in functions and conversions, callable expressions can appear in statement context.
        if ( auto identifiedBase = dynamic_cast<IdentifierAsExpression*>(functionCall->base)) {
            if (identifiedBase->identifier != "append" && identifiedBase->identifier != "len" && !TypeEntity::IsBuiltInType(identifiedBase->identifier))
            return;
        }
    }
    
    semantic->addError(node->expression->name() + " expression not available for statement");
}

void TypesVisitor::onFinishVisit(WhileStatement* node) {
    if (node->conditionExpression->typeExpression->type != TypeEntity::Boolean) {
        semantic->addError("The non-bool value used as a condition in loop");
    }
}

void TypesVisitor::onFinishVisit(IfStatement* node) {
    if (node->condition->typeExpression->type != TypeEntity::Boolean) {
        semantic->addError("The non-bool value used as a condition in if statement");
    }
}

void TypesVisitor::onFinishVisit(SwitchStatement* node) {
    TypeEntity* typeSwitchExpression = node->expression->typeExpression;

    int index = 0;
    for (auto caseClause : node->clauseList) {

        if (caseClause->expressionCase) {
            TypeEntity* caseExpressionType = caseClause->expressionCase->typeExpression;

            if (caseExpressionType->equal(typeSwitchExpression)) {
                caseClause->expressionCase->typeExpression = typeSwitchExpression;
                
            } else {
                semantic->addError("The type of expression in case " + std::to_string(index) + " statement should be the same as in switch");
            }
        }

        index++;
    }
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

bool TypesVisitor::definePrintsFunctions(CallableExpression* function) {
    std::vector<TypeEntity::TypeEntityEnum> printableTypes = {
            TypeEntity::Int,
            TypeEntity::UntypedInt,
            TypeEntity::Float,
            TypeEntity::UntypedFloat,
            TypeEntity::Boolean,
            TypeEntity::String,
            TypeEntity::Array,
        };

    if (function->arguments.size() == 1) {
        auto typeArgument = function->arguments.front()->typeExpression;

        auto equal = std::find_if(printableTypes.begin(), printableTypes.end(), 
                        [typeArgument](TypeEntity::TypeEntityEnum type){ return type == typeArgument->type; });

        if (equal != printableTypes.end()) {
            function->typeExpression = new TypeEntity(TypeEntity::Void);
            return true;

        } else {
            semantic->addError("The invalid print/println function argument");
        }

    } else {
        semantic->addError("The print/println functions accept only one argument");
    }

    return false;
}

bool TypesVisitor::defineLenFunction(CallableExpression* function) {
    std::vector<TypeEntity::TypeEntityEnum> lenableTypes = {
        TypeEntity::String,
        TypeEntity::Array
    };

    if (function->arguments.size() == 1) {
        auto typeArgument = function->arguments.front()->typeExpression;

        auto equal = std::find_if(lenableTypes.begin(), lenableTypes.end(), 
                        [typeArgument](TypeEntity::TypeEntityEnum type){ return type == typeArgument->type; });

        if (equal != lenableTypes.end()) {
            function->typeExpression = new TypeEntity(TypeEntity::Int);

            return true;

        } else {
            semantic->addError("The invalid len function argument");
        }

    } else {
        semantic->addError("The len function accept only one argument");
    }

    return false;
}

bool TypesVisitor::defineAppendFunction(CallableExpression* function) {
    if (function->arguments.size() == 2) {
        auto arrayArgType = function->arguments.front()->typeExpression;
        auto newElementArgType = (*(++function->arguments.begin()))->typeExpression;

        if (arrayArgType->type == TypeEntity::Array && std::get<ArraySignatureEntity*>(arrayArgType->value)->elementType->equal(newElementArgType)) {
            function->typeExpression = arrayArgType;
            return true;

        } else {
            semantic->addError("The invalid append function arguments");
        }

    } else {
        semantic->addError("The append function accepts two arguments");
    }

    return false;
}

bool TypesVisitor::defineReadFunction(CallableExpression* function, TypeEntity::TypeEntityEnum type) {
    if (function->arguments.size() == 0) {
        function->typeExpression = new TypeEntity(type);
        return true;
    }
    
    semantic->addError("The readable functions not accepts arguments");
    return false;
}

bool TypesVisitor::defineTypeBuiltInFunction(CallableExpression* function) {
    
    if (auto idFunctionBase = dynamic_cast<IdentifierAsExpression*>(function->base)) {

        if (idFunctionBase->identifier == "len") {
            return defineLenFunction(function);

        } else if (idFunctionBase->identifier == "print") {
            return definePrintsFunctions(function);

        } else if (idFunctionBase->identifier == "println") {
            return definePrintsFunctions(function);

        } else if (idFunctionBase->identifier == "readInt" && function->arguments.empty()) {
            return defineReadFunction(function, TypeEntity::Int);

        } else if (idFunctionBase->identifier == "readFloat" && function->arguments.empty()) {
            return defineReadFunction(function, TypeEntity::Float);

        } else if (idFunctionBase->identifier == "readString" && function->arguments.empty()) {
            return defineReadFunction(function, TypeEntity::String);
            
        } else if (idFunctionBase->identifier == "readBool" && function->arguments.empty()) {
            return defineReadFunction(function, TypeEntity::Boolean);
            
        } else if (idFunctionBase->identifier == "append") {
            return defineAppendFunction(function);
        }
    }

    return false;
}