#include <unordered_map>
#include "semantic.h"

const std::unordered_map<std::string, TypeEntity*> Semantic::BuiltInFunctions = {
    {"print", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"println", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"len", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Any)))})),
        new TypeEntity(TypeEntity::Int)))
    },

    {"scan", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"append", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({
            new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Any))),
            new TypeEntity(TypeEntity::Any)
        })),
        new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Any)))))
    },

    // Conversions
    {"int", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::Int)))
    },

    {"float32", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::Float)))
    },

    {"float64", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::Float)))
    },

    {"string", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Any)})),
        new TypeEntity(TypeEntity::String)))
    },
};