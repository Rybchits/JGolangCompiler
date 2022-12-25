#include "./type_check_visitor.h"
#include <unordered_map>
#include <iostream>

bool TypeCheckVisitor::check(NodeAST* root) {
    std::cout << "____Start type check____" << std::endl;
    scopesDeclarations.push_back(std::unordered_map<std::string, JavaType*>());

    // Added package function
    for (auto function : semantic->classes[Semantic::GlobalClassName]->getMethods()) {
        scopesDeclarations.back()[function.first] = function.second->toJavaType();
    }

    root->acceptVisitor(this);

    for (const auto & [ key, value ] : typesExpressions) {
        std::cout << key << ": " << value->toByteCode() << std::endl;
    }

    return true;
}

void TypeCheckVisitor::onStartVisit(BlockStatement* node) {
    if (!lastAddedScopeInFuncDecl) {
        scopesDeclarations.push_back(std::unordered_map<std::string, JavaType*>());
    }
    lastAddedScopeInFuncDecl = false;
}

void TypeCheckVisitor::onFinishVisit(BlockStatement* node) {
    scopesDeclarations.pop_back();
}

void TypeCheckVisitor::onStartVisit(FunctionDeclaration* node) {
    scopesDeclarations.push_back(std::unordered_map<std::string, JavaType*>());

    // Get arguments from current JavaFunction
    auto methods = semantic->classes[Semantic::GlobalClassName]->getMethods();
    JavaFunction* currentFunction = methods[node->identifier];

    for (auto arg : currentFunction->getArguments()) {
        scopesDeclarations.back()[arg.first] = arg.second;
    }

    lastAddedScopeInFuncDecl = true;
}

void TypeCheckVisitor::onFinishVisit(VariableDeclaration* node) {

    if (node->identifiersWithType->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->errors.push_back("Assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();
        for (auto id : node->identifiersWithType->identifiers) {

            if (JavaType::IsBuiltInType(id)) {
                semantic->errors.push_back("Variable " + id + " collides with the 'builtin' type");
            }
            
            if (node->identifiersWithType->type != nullptr) {
                auto generalType = new JavaType(node->identifiersWithType->type);

                // Compare types expressions with the declared type
                if (node->values.size() != 0 && typesExpressions[(*currentValue)->nodeId]->equal(generalType))
                    scopesDeclarations.back()[id] = generalType;

                else
                    semantic->errors.push_back("Assignment variable " + id + "must have equals types");

                currentValue++;
                
            } else {
                if (typesExpressions[(*currentValue)->nodeId]->type == JavaType::UntypedFloat) {
                    scopesDeclarations.back()[id] = new JavaType(JavaType::Float);

                } else if (typesExpressions[(*currentValue)->nodeId]->type == JavaType::UntypedInt) {
                    scopesDeclarations.back()[id] = new JavaType(JavaType::Int);

                } else {
                    scopesDeclarations.back()[id] = typesExpressions[(*currentValue)->nodeId];
                }
            }
            std::cout << id << ": index scope " << scopesDeclarations.size() << " " << scopesDeclarations.back()[id]->toByteCode() << std::endl;
        }
    }
}

void TypeCheckVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    
    if (node->identifiers.size() != node->values.size() && node->values.size() != 0) {
        semantic->errors.push_back("Short variable declaration: assignment count mismatch");

    } else {
        auto currentValue = node->values.begin();

        for (auto id : node->identifiers) {
            if (JavaType::IsBuiltInType(id)) {
                semantic->errors.push_back("Variable " + id + " collides with the 'builtin' type");
            }

            if (typesExpressions[(*currentValue)->nodeId]->type == JavaType::UntypedFloat) {
                scopesDeclarations.back()[id] = new JavaType(JavaType::Float);
                    
            } else if (typesExpressions[(*currentValue)->nodeId]->type == JavaType::UntypedInt) {
                scopesDeclarations.back()[id] = new JavaType(JavaType::Int);

            } else {
                scopesDeclarations.back()[id] = typesExpressions[(*currentValue)->nodeId];
            }

            currentValue++;
            std::cout << id << ": index scope " << scopesDeclarations.size() << " " << scopesDeclarations.back()[id]->toByteCode() << std::endl;
        }
    }
}

void TypeCheckVisitor::onFinishVisit(IdentifierAsExpression* node) {

    for (auto scope = scopesDeclarations.rbegin(); scope != scopesDeclarations.rend(); ++scope) {

        if ((*scope).count(node->identifier)) {
            typesExpressions[node->nodeId] = (*scope)[node->identifier];
            return ;
        }
    }
    
    semantic->errors.push_back("Undefined: " + node->identifier);
    typesExpressions[node->nodeId] = new JavaType();
}


void TypeCheckVisitor::onFinishVisit(IntegerExpression* node) {
    typesExpressions[node->nodeId] = new JavaType(JavaType::UntypedInt);
}

void TypeCheckVisitor::onFinishVisit(BooleanExpression* node) {
    typesExpressions[node->nodeId] = new JavaType(JavaType::Boolean);
}

void TypeCheckVisitor::onFinishVisit(FloatExpression* node) {
    typesExpressions[node->nodeId] = new JavaType(JavaType::UntypedFloat);
}

void TypeCheckVisitor::onFinishVisit(StringExpression* node) {
    typesExpressions[node->nodeId] = new JavaType(JavaType::String);
}

void TypeCheckVisitor::onFinishVisit(RuneExpression* node) {
    typesExpressions[node->nodeId] = new JavaType(JavaType::Rune);
}

void TypeCheckVisitor::onFinishVisit(NilExpression* node) {
    typesExpressions[node->nodeId] = new JavaType();
}

void TypeCheckVisitor::onFinishVisit(UnaryExpression* node) {
    if (typesExpressions[node->expression->nodeId]->type == JavaType::Invalid) {
        typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        return ;
    }

    if (node->type == UnaryExpressionEnum::UnaryNot) {
        if (typesExpressions[node->expression->nodeId]->type == JavaType::Boolean) {
            typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        } else {
            typesExpressions[node->nodeId] = new JavaType();
            semantic->errors.push_back(node->name() + " must have boolean expression");
        }

    } else if (node->type == UnaryExpressionEnum::Variadic) {
        if (typesExpressions[node->expression->nodeId]->type == JavaType::Array) {
            typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        } else {
            typesExpressions[node->nodeId] = new JavaType();
            semantic->errors.push_back(node->name() + " must have array expression");
        }

    } else {
        if (typesExpressions[node->expression->nodeId]->isNumeric()) {
            typesExpressions[node->nodeId] = typesExpressions[node->expression->nodeId];
        } else {
            typesExpressions[node->nodeId] = new JavaType();
            semantic->errors.push_back(node->name() + " must have numeric expression");
        }
    }
}

void TypeCheckVisitor::onFinishVisit(BinaryExpression* node) {
    if (typesExpressions[node->lhs->nodeId]->type == JavaType::Invalid) {
        typesExpressions[node->nodeId] = typesExpressions[node->lhs->nodeId];
        return ;

    } else if (typesExpressions[node->rhs->nodeId]->type == JavaType::Invalid) {
        typesExpressions[node->nodeId] = typesExpressions[node->rhs->nodeId];
        return ;
    }

    if (node->type == Addition || node->type == Subtraction || node->type == Multiplication || node->type == Division || node->type == Mod) {
        if (typesExpressions[node->lhs->nodeId]->isNumeric() && typesExpressions[node->rhs->nodeId]->isNumeric()) {
            typesExpressions[node->nodeId] = typesExpressions[node->lhs->nodeId]->determinePriorityType(typesExpressions[node->rhs->nodeId]);

        } else {
            typesExpressions[node->nodeId] = new JavaType();
            semantic->errors.push_back(node->name() + " must have numeric expressions");
        }

    } else if (node->type == Or || node->type == And) {
        if (typesExpressions[node->lhs->nodeId]->type == JavaType::Boolean && typesExpressions[node->rhs->nodeId]->type == JavaType::Boolean) {
            typesExpressions[node->nodeId] = typesExpressions[node->lhs->nodeId]->determinePriorityType(typesExpressions[node->rhs->nodeId]);
        } else {
            typesExpressions[node->nodeId] = new JavaType();
            semantic->errors.push_back(node->name() + " must have boolean expressions");
        }
    } else {
        if (typesExpressions[node->lhs->nodeId]->equal(typesExpressions[node->rhs->nodeId]) 
            && typesExpressions[node->lhs->nodeId]->type != JavaType::Array 
            && typesExpressions[node->lhs->nodeId]->type != JavaType::UserType) {
            typesExpressions[node->nodeId] = typesExpressions[node->lhs->nodeId]->determinePriorityType(typesExpressions[node->rhs->nodeId]);
        } else {
            typesExpressions[node->nodeId] = new JavaType();
            semantic->errors.push_back(node->name() + " must have equals types expressions. Comparison of arrays and functions are'nt supported");
        }
    }
}

void TypeCheckVisitor::onFinishVisit(CallableExpression* node) {
    // TODO
    // Проверить является ли название вызываемого exp встроенным типом
        // Проверить передаваемый тип (он один)
        // Вернуть 

    // Call declarated function
    JavaType* baseType = typesExpressions[node->base->nodeId];
    if (baseType->type == JavaType::Function) {

        std::list<ExpressionAST*>::const_iterator argExprType = node->arguments.begin();
        auto signature = std::get<JavaFunctionSignature*>(baseType->value);

        int index = 0;
        for (auto argType : signature->argsTypes) {
            if (!argType->equal(typesExpressions[(*argExprType)->nodeId])) {
                semantic->errors.push_back("Cannot use expression index " + std::to_string(index) + " in argument");
                return;
            }
            index++;
            argExprType++;
        }
        typesExpressions[node->nodeId] = signature->returnType;
        return ;

    } else if (baseType->type == JavaType::Invalid) {
        return ;
    }

    semantic->errors.push_back("Cannot call non-function");
    typesExpressions[node->nodeId] = new JavaType();
}

void TypeCheckVisitor::onFinishVisit(AccessExpression* node) {
    if (node->type == AccessExpressionEnum::Indexing && typesExpressions[node->accessor->nodeId]->type == JavaType::Array) {
        typesExpressions[node->nodeId] = std::get<JavaArraySignature*>(typesExpressions[node->accessor->nodeId]->value)->type;
        return ;

    } else if (node->type == AccessExpressionEnum::FieldSelect && typesExpressions[node->accessor->nodeId]->type == JavaType::UserType) {
        // struct aren't supported yet
    }

    typesExpressions[node->nodeId] = new JavaType();
}

void TypeCheckVisitor::onFinishVisit(CompositeLiteral* node) {
    if (auto arrayType = dynamic_cast<ArraySignature*>(node->type)) {
        auto declaredElementType = new JavaType(arrayType->arrayElementType);

        int index = 0;
        for (auto element : node->elements) {
            if (!typesExpressions[element->nodeId]->equal(declaredElementType)) {
                semantic->errors
                    .push_back("Array declarated type mismatch " + std::string("index ") + std::to_string(index));

                typesExpressions[node->nodeId] = new JavaType();
                return;
            }
            index++;
        }

        typesExpressions[node->nodeId] = new JavaType(arrayType);
    }
}

void TypeCheckVisitor::onFinishVisit(ElementCompositeLiteral* node) {

    if (std::holds_alternative<ExpressionAST *>(node->value)) {
        typesExpressions[node->nodeId] = typesExpressions[std::get<ExpressionAST*>(node->value)->nodeId];

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(node->value)) {
        JavaType* tentativeElementType = nullptr;

        for (auto el : std::get<std::list<ElementCompositeLiteral *>>(node->value)) {
            if (tentativeElementType == nullptr)
                tentativeElementType = typesExpressions[el->nodeId];
            else {
                tentativeElementType = tentativeElementType->determinePriorityType(typesExpressions[el->nodeId]);
            }
        }

        if (tentativeElementType->type == JavaType::Invalid) {
            typesExpressions[node->nodeId] = tentativeElementType;

        } else {
            typesExpressions[node->nodeId] = new JavaType(new JavaArraySignature(tentativeElementType));
        }
    }
}

