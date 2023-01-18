#include "visitors/types_visitor.h"

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
        auto typeArgument = typesExpressions[function->arguments.front()->nodeId];

        auto equal = std::find_if(printableTypes.begin(), printableTypes.end(), 
                        [typeArgument](TypeEntity::TypeEntityEnum type){ return type == typeArgument->type; });

        if (equal != printableTypes.end()) {
            typesExpressions[function->nodeId] = new TypeEntity(TypeEntity::Void);
            return true;

        } else {
            semantic->errors.push_back("The invalid print/println function argument");
        }

    } else {
        semantic->errors.push_back("The print/println functions accept only one argument");
    }

    return false;
}

bool TypesVisitor::defineLenFunction(CallableExpression* function) {
    std::vector<TypeEntity::TypeEntityEnum> lenableTypes = {
        TypeEntity::String,
        TypeEntity::Array
    };

    if (function->arguments.size() == 1) {
        auto typeArgument = typesExpressions[function->arguments.front()->nodeId];

        auto equal = std::find_if(lenableTypes.begin(), lenableTypes.end(), 
                        [typeArgument](TypeEntity::TypeEntityEnum type){ return type == typeArgument->type; });

        if (equal != lenableTypes.end()) {
            typesExpressions[function->nodeId] = new TypeEntity(TypeEntity::Int);

            return true;

        } else {
            semantic->errors.push_back("The invalid len function argument");
        }

    } else {
        semantic->errors.push_back("The len function accept only one argument");
    }

    return false;
}

bool TypesVisitor::defineAppendFunction(CallableExpression* function) {
    if (function->arguments.size() == 2) {
        auto arrayArgType = typesExpressions[function->arguments.front()->nodeId];
        auto newElementArgType = typesExpressions[(*(++function->arguments.begin()))->nodeId];

        if (arrayArgType->type == TypeEntity::Array && std::get<ArraySignatureEntity*>(arrayArgType->value)->elementType->equal(newElementArgType)) {

            typesExpressions[function->nodeId] = arrayArgType;

            return true;

        } else {
            semantic->errors.push_back("The invalid append function arguments");
        }

    } else {
        semantic->errors.push_back("The append function accepts two arguments");
    }

    return false;
}

bool TypesVisitor::defineReadFunction(CallableExpression* function, TypeEntity::TypeEntityEnum type) {
    if (function->arguments.size() == 0) {
        typesExpressions[function->nodeId] = new TypeEntity(type);
        return true;
    }
    
    semantic->errors.push_back("The readable functions not accepts arguments");
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