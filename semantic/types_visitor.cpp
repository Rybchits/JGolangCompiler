#include "./types_visitor.h"
#include "./builtins.h"

#include <unordered_map>
#include <map>
#include <iostream>

void TypesVisitor::analyzePackageClass(ClassEntity* packageClass, std::vector<std::string>& idsConstants) {
    
    // Added signatures functions in scope
    for (auto & [methodName, methodSignature] : packageClass->getMethods()) {
        scopesDeclarations.add(methodName, VariableEntity(methodSignature->toTypeEntity()));
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
                addError("Cannot initialize " + variableName);
                
            } else if (isConstVariable && !constCheckVisitor.isConstExpression(variableEntity->declaration)) {
                addError("Expression is not constant");

            } else {
                scopesDeclarations.add(variableName, VariableEntity(typeExpression, isConstVariable));
                variableEntity->type = typeExpression;
            }
        }
    }

    // Analize methods
    for (auto & [methodName, methodSignature] : packageClass->getMethods()) {
        currentMethodEntity = methodSignature.get();

        scopesDeclarations.pushScope();

        // Get arguments from current MethodEntity
        for (auto & [id, type] : methodSignature->getArguments()) {
            scopesDeclarations.add(id, VariableEntity(type, false, true));
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

        if (var.numberUsage == 0 && !var.isArgument && !var.isConst) {
            addError("Unused variable " + id);
        }
    }

    scopesDeclarations.popScope();
}

void TypesVisitor::onFinishVisit(VariableDeclaration* node) {

    if (node->identifiersWithType->identifiers.size() != node->values.size() && node->values.size() != 0) {
        addError("Assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();
        for (const auto& id : node->identifiersWithType->identifiers) {
            if (id != "_") numberLocalVariables++;

            if (scopesDeclarations.findAtLastScope(id) != nullptr) {
                addError(id + " redeclared in this block");
                continue;
            }

            if (TypeEntity::IsBuiltInType(id)) {
                addError("Variable " + id + " collides with the 'builtin' type");
                continue;
            }

            // Const checking
            if (node->isConst && (*currentValue)->typeExpression->type == TypeEntity::Array) {
                addError("Go does not support constant arrays, maps or slices");

            } else if (node->isConst && !constCheckVisitor.isConstExpression(currentValue->get())) {
                addError("Cannot assignment not const expression for " + id);
            }
            
            if (node->identifiersWithType->type != nullptr) {
                auto generalType = std::make_shared<const TypeEntity>(node->identifiersWithType->type.get());

                // Compare types expressions with the declared type
                if (node->values.size() != 0 ) {
                    if ((*currentValue)->typeExpression->equal(generalType)) {
                        (*currentValue)->typeExpression = determinePriorityType((*currentValue)->typeExpression, generalType);
                        scopesDeclarations.add(id, VariableEntity(generalType, node->isConst));

                    } else {
                        addError("Assignment variable " + id + " must have equals types");
                    }
                } else {
                    scopesDeclarations.add(id, VariableEntity(generalType, node->isConst));
                }
                
            } else {
                if ((*currentValue)->typeExpression->type == TypeEntity::UntypedFloat) {
                    scopesDeclarations.add(id, VariableEntity(std::make_shared<const TypeEntity>(TypeEntity::Float), node->isConst));

                } else if ((*currentValue)->typeExpression->type == TypeEntity::UntypedInt) {
                    scopesDeclarations.add(id, VariableEntity(std::make_shared<const TypeEntity>(TypeEntity::Int), node->isConst));

                } else {
                    scopesDeclarations.add(id, VariableEntity((*currentValue)->typeExpression, node->isConst));
                }
            }

            currentValue++;
        }
    }
}

void TypesVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    
    if (node->identifiers.size() != node->values.size() && node->values.size() != 0) {
        addError("Short variable declaration: assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();

        for (const auto& id : node->identifiers) {
            if (id != "_") numberLocalVariables++;

            if (TypeEntity::IsBuiltInType(id)) {
                addError("Variable " + id + " collides with the 'builtin' type");
            }

            if ((*currentValue)->typeExpression->type == TypeEntity::UntypedFloat) {
                scopesDeclarations.add(id, VariableEntity(std::make_shared<const TypeEntity>(TypeEntity::Float)));
                    
            } else if ((*currentValue)->typeExpression->type == TypeEntity::UntypedInt) {
                scopesDeclarations.add(id, VariableEntity(std::make_shared<const TypeEntity>(TypeEntity::Int)));

            } else {
                scopesDeclarations.add(id, VariableEntity((*currentValue)->typeExpression));
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

    } else if (IsBuiltInFunction(node->identifier)) {
        node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::BuiltInFunction, node->identifier);
        return;

    } else if (node->identifier == "_" && node->isDestination) {
        node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::Any);
        return;
    }
    
    std::string errorMessage = "Undefined: " + node->identifier;

    if (node->identifier == "_" && !node->isDestination) {
        errorMessage = "Cannot use _ as value";
    }

    addError(errorMessage);
    node->typeExpression = std::make_shared<const TypeEntity>();
}


void TypesVisitor::onFinishVisit(IntegerExpression* node) {
    node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::UntypedInt);
}


void TypesVisitor::onFinishVisit(BooleanExpression* node) {
    node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::Boolean);
}


void TypesVisitor::onFinishVisit(FloatExpression* node) {
    node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::UntypedFloat);
}


void TypesVisitor::onFinishVisit(StringExpression* node) {
    node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::String);
}


void TypesVisitor::onFinishVisit(NilExpression* node) {
    node->typeExpression = std::make_shared<const TypeEntity>();
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
            node->typeExpression = std::make_shared<const TypeEntity>();
            addError(node->name() + " must have boolean expression");
        }

    } else if (node->type == UnaryExpression::Variadic) {
        if (node->expression->typeExpression->type == TypeEntity::Array) {
            node->typeExpression = node->expression->typeExpression;
        } else {
            node->typeExpression = std::make_shared<const TypeEntity>();
            addError(node->name() + " must have array expression");
        }

    } else {
        if (node->expression->typeExpression->isNumeric()) {
            node->typeExpression = node->expression->typeExpression;
        } else {
            node->typeExpression = std::make_shared<const TypeEntity>();
            addError(node->name() + " must have numeric expression");
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
        node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::String);

    } else if (node->type == BinaryExpression::Addition || node->type == BinaryExpression::Subtraction 
    || node->type == BinaryExpression::Multiplication || node->type == BinaryExpression::Division || node->type == BinaryExpression::Mod) {

        if (leftExprType->isNumeric() && rightExprType->isNumeric() && leftExprType->equal(rightExprType)) {
            auto resultTypeExression = determinePriorityType(leftExprType, rightExprType);

            node->typeExpression = resultTypeExression;
            node->lhs->typeExpression = resultTypeExression;
            node->rhs->typeExpression = resultTypeExression;

        } else {
            node->typeExpression = std::make_shared<const TypeEntity>();
            addError(node->name() + " must have same numeric types expressions");
        }

    } else if (node->type == BinaryExpression::Or || node->type == BinaryExpression::And) {
        if (leftExprType->type == TypeEntity::Boolean && rightExprType->type == TypeEntity::Boolean) {
            node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::Boolean);
        } else {
            node->typeExpression = std::make_shared<const TypeEntity>();
            addError(node->name() + " must have boolean expressions");
        }
    } else {
        if (leftExprType->equal(rightExprType) && (leftExprType->isFloat()
            || leftExprType->isInteger()
            || leftExprType->type == TypeEntity::String
            || ((leftExprType->type == TypeEntity::Boolean || leftExprType->type == TypeEntity::Array) 
            && (node->type == BinaryExpression::Equal || node->type == BinaryExpression::NotEqual)))
        ) {
            node->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::Boolean);

        } else {
            node->typeExpression = std::make_shared<const TypeEntity>();
            addError(node->name() + " must have equal types of expressions. Comparison of booleans, arrays and functions are'nt supported");
        }
    }
}


void TypesVisitor::onFinishVisit(CallableExpression* node) {

    // Call declarated function
    TypePtr baseType = node->base->typeExpression;

    if (baseType->type == TypeEntity::Function) {

        ExpressionList::const_iterator argExprType = node->arguments.begin();
        const auto& signature = std::get<FunctionSignatureEntity>(baseType->value);

        if (signature.argsTypes.size() != node->arguments.size()) {
            addError("Invalid number of arguments");
            node->typeExpression = std::make_shared<const TypeEntity>();
            return;
        }

        int index = 0;
        for (const auto& argType : signature.argsTypes) {
            if (!argType->equal((*argExprType)->typeExpression)) {
                node->typeExpression = std::make_shared<const TypeEntity>();
                addError("Cannot use expression index " + std::to_string(index) + " in argument");
                return;

            } else {
                (*argExprType)->typeExpression = determinePriorityType((*argExprType)->typeExpression, argType);
            }
            index++;
            argExprType++;
        }
        
        node->typeExpression = signature.returnType;
        return ;
    
    } else if (baseType->type == TypeEntity::BuiltInFunction) {
        bool isCheckBuiltInFunctionSuccess = defineTypeBuiltInFunction(node);

        if (!isCheckBuiltInFunctionSuccess) {
            node->typeExpression = std::make_shared<const TypeEntity>();
        }
        return ;
    }

    addError("Cannot call non-function");
    node->typeExpression = std::make_shared<const TypeEntity>();
}


void TypesVisitor::onFinishVisit(AccessExpression* node) {
    switch (node->type)
    {
    case AccessExpression::Indexing:
        if (node->base->typeExpression->type != TypeEntity::Array) {
            addError("Base for indexing must be array");

        } else if (!node->accessor->typeExpression->isInteger()) {
            addError("Index must be integer value");

        } else {
            node->typeExpression = std::get<ArraySignatureEntity>(node->base->typeExpression->value).elementType;
            return ;
        }
        node->typeExpression = std::make_shared<const TypeEntity>();
        break;

    case AccessExpression::FieldSelect:
        break;
    
    default:
        break;
    }

    node->typeExpression = std::make_shared<const TypeEntity>();
}

void TypesVisitor::onStartVisit(CompositeLiteral* node) {

    // Если это массив, нам нужно записать его тип и запомнить id узла для проверки элементов
    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type.get())) {
        node->typeExpression = std::make_shared<const TypeEntity>(arrayType);
        typeCurrentArray = node->typeExpression;
        indexCurrentAxisArray = 0;
    }
}

void TypesVisitor::onStartVisit(ElementCompositeLiteral* node) {
    indexCurrentAxisArray++;
}


void TypesVisitor::onFinishVisit(CompositeLiteral* node) {

    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type.get())) {

        if (arrayType->dimensions < node->elements.size()) {
            addError("Array has more elements than declarated");
            node->typeExpression = std::make_shared<const TypeEntity>();
            return;
        }

        // Тип текущего массива был вычислен при первом заходе в этот узел
        auto declaredElementType = std::get<ArraySignatureEntity>(node->typeExpression->value).elementType;

        int index = 0;
        for (const auto& element : node->elements) {
            if (!element->typeExpression->equal(declaredElementType)) {
                addError("Array declarated type mismatch " + std::string("index ") + std::to_string(index));

                node->typeExpression = std::make_shared<const TypeEntity>();
            }
            index++;
        }

        typeCurrentArray = nullptr;
        indexCurrentAxisArray = -1;
    }
}


void TypesVisitor::onFinishVisit(ElementCompositeLiteral* node) {

    TypePtr declaratedElementType =
        typeAxis(typeCurrentArray, indexCurrentAxisArray);

    if (std::holds_alternative<ExpressionASTPtr>(node->value)) {
        const auto& expression = std::get<ExpressionASTPtr>(node->value);

        if (declaratedElementType->equal(expression->typeExpression)) {
            expression->typeExpression = determinePriorityType(declaratedElementType, expression->typeExpression);
            node->typeExpression = declaratedElementType;

        } else {
            addError("Expression at " + std::to_string(indexCurrentAxisArray) + " axis has invalid type");
            node->typeExpression = std::make_shared<const TypeEntity>();
        }

    } else if (std::holds_alternative<ElementCompositeLiteralList>(node->value)) {

        if (const auto* declaratedTypeAxis = std::get_if<ArraySignatureEntity>(&declaratedElementType->value)) {

            if (declaratedTypeAxis->dims < std::get<ElementCompositeLiteralList>(node->value).size() ) {
                addError("Array at " + std::to_string(indexCurrentAxisArray) + " axis has many values");
                node->typeExpression = std::make_shared<const TypeEntity>();

            } else {
                node->typeExpression = declaratedElementType;
            }

        } else {
            addError("Expression at " + std::to_string(indexCurrentAxisArray) + " axis has invalid type");
            node->typeExpression = std::make_shared<const TypeEntity>();
        }
    }

    indexCurrentAxisArray--;
}


void TypesVisitor::onFinishVisit(AssignmentStatement* node) {
    
    // Check const variables
    for (const auto& var : node->lhs) {
        if (auto idVariable = dynamic_cast<IdentifierAsExpression*>(var.get())) {
                    
            VariableEntity* variable = scopesDeclarations.find(idVariable->identifier);

            if (variable != nullptr && variable->isConst) {
                addError("Cannot assign to const " + idVariable->identifier);
            }
            
        } else if (dynamic_cast<AccessExpression*>(var.get()) == nullptr) {
            addError("Cannot assign to " + var->name());
        }
    }

    if (node->type == AssignmentStatement::SimpleAssign) {

        if (node->lhs.size() != node->rhs.size()) {
            addError(
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
                        addError("Value by index " + std::to_string(index) + std::string(" cannot be represented for assignment"));

                    } else {
                        (*valueIterator)->typeExpression = determinePriorityType((*valueIterator)->typeExpression, (*idIterator)->typeExpression);
                    }

                } else {

                    // Indexing access expression must have the base as array type
                    if ((*idIterator)->typeExpression->type == TypeEntity::Array) {

                        TypePtr typeElement = std::get<ArraySignatureEntity>((*idIterator)->typeExpression->value).elementType;

                        if (!typeElement->equal((*valueIterator)->typeExpression)) {
                            addError("Value by index " + std::to_string(index) + std::string(" cannot be represented for assignment"));

                        } else if (!(*indexIterator)->typeExpression->isInteger()) {
                            addError("Index must be integer value");

                        } else {
                            (*valueIterator)->typeExpression = determinePriorityType((*valueIterator)->typeExpression, typeElement);
                        }
                    } else {
                        addError("Cannot get value by index. Not array");
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
        addError("Return cannot take more than one value");

    } else if (currentMethodEntity->getReturnType()->type != TypeEntity::Void && node->returnValues.empty()) {
        addError("Missing return value");
    }

    for (const auto& value : node->returnValues) {
        if (!currentMethodEntity->getReturnType()->equal(value->typeExpression)) {
            addError("Cannot use this value for return");
        }
    }
}


void TypesVisitor::onStartVisit(ExpressionStatement* node) {
    // Increment and decrement can be statements
    if (auto unaryExpression = dynamic_cast<UnaryExpression*>(node->expression.get())) {
        if (unaryExpression->type == UnaryExpression::Decrement || unaryExpression->type == UnaryExpression::Increment) {
            return;
        }
        
    } else if (auto functionCall = dynamic_cast<CallableExpression*>(node->expression.get())) {
        // With the exception of specific built-in functions and conversions, callable expressions can appear in statement context.
        if ( auto identifiedBase = dynamic_cast<IdentifierAsExpression*>(functionCall->base.get())) {
            if (identifiedBase->identifier != "append" && identifiedBase->identifier != "len" && !TypeEntity::IsBuiltInType(identifiedBase->identifier))
            return;
        }
    }
    
    addError(node->expression->name() + " expression not available for statement");
}

void TypesVisitor::onFinishVisit(WhileStatement* node) {
    if (node->conditionExpression->typeExpression->type != TypeEntity::Boolean) {
        addError("The non-bool value used as a condition in loop");
    }
}

void TypesVisitor::onFinishVisit(IfStatement* node) {
    if (node->condition->typeExpression->type != TypeEntity::Boolean) {
        addError("The non-bool value used as a condition in if statement");
    }
}

void TypesVisitor::onFinishVisit(SwitchStatement* node) {
    TypePtr typeSwitchExpression = node->expression->typeExpression;

    int index = 0;
    for (const auto& caseClause : node->clauseList) {

        if (caseClause->expressionCase) {
            TypePtr caseExpressionType = caseClause->expressionCase->typeExpression;

            if (caseExpressionType->equal(typeSwitchExpression)) {
                caseClause->expressionCase->typeExpression = typeSwitchExpression;
                
            } else {
                addError("The type of expression in case " + std::to_string(index) + " statement should be the same as in switch");
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
            function->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::Void);
            return true;

        } else {
            addError("The invalid print/println function argument");
        }

    } else {
        addError("The print/println functions accept only one argument");
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
            function->typeExpression = std::make_shared<const TypeEntity>(TypeEntity::Int);

            return true;

        } else {
            addError("The invalid len function argument");
        }

    } else {
        addError("The len function accept only one argument");
    }

    return false;
}

bool TypesVisitor::defineAppendFunction(CallableExpression* function) {
    if (function->arguments.size() == 2) {
        auto arrayArgType = function->arguments.front()->typeExpression;
        auto newElementArgType = (*(++function->arguments.begin()))->typeExpression;

        if (arrayArgType->type == TypeEntity::Array && std::get<ArraySignatureEntity>(arrayArgType->value).elementType->equal(newElementArgType)) {
            function->typeExpression = arrayArgType;
            return true;

        } else {
            addError("The invalid append function arguments");
        }

    } else {
        addError("The append function accepts two arguments");
    }

    return false;
}

bool TypesVisitor::defineReadFunction(CallableExpression* function, TypeEntity::TypeEntityEnum type) {
    if (function->arguments.size() == 0) {
        function->typeExpression = std::make_shared<const TypeEntity>(type);
        return true;
    }
    
    addError("The readable functions not accepts arguments");
    return false;
}

bool TypesVisitor::defineTypeBuiltInFunction(CallableExpression* function) {
    
    if (auto idFunctionBase = dynamic_cast<IdentifierAsExpression*>(function->base.get())) {

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