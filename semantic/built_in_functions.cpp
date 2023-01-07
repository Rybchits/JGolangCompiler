#include <unordered_map>
#include "semantic.h"

const std::unordered_map<std::string, TypeEntity*> Semantic::BuiltInFunctions = {
    {"printInt", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Int)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"printFloat", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Float)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"printString", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::String)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"printBoolean", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Boolean)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"printRune", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::Int)})),
        new TypeEntity(TypeEntity::Void)))
    },

    {"lenArray", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Any)))})),
        new TypeEntity(TypeEntity::Int)))
    },

    {"lenString", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({new TypeEntity(TypeEntity::String)})),
        new TypeEntity(TypeEntity::Int)))
    },

    {"readInt", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>()), new TypeEntity(TypeEntity::Int)))
    },

    {"readFloat", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>()), new TypeEntity(TypeEntity::Float)))
    },

    {"readString", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>()), new TypeEntity(TypeEntity::String)))
    },

    {"readBool", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>()), new TypeEntity(TypeEntity::Boolean)))
    },

    {"appendIntArray", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({
            new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Int))),
            new TypeEntity(TypeEntity::Int)
        })),
        new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Int)))))
    },

    {"appendFloatArray", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({
            new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Float))),
            new TypeEntity(TypeEntity::Float)
        })),
        new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Float)))))
    },

    {"appendStirngArray", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({
            new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::String))),
            new TypeEntity(TypeEntity::String)
        })),
        new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::String)))))
    },

    {"appendBooleanArray", new TypeEntity(new FunctionSignatureEntity(
        *(new std::list<TypeEntity*>({
            new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Boolean))),
            new TypeEntity(TypeEntity::Boolean)
        })),
        new TypeEntity(new ArraySignatureEntity(new TypeEntity(TypeEntity::Boolean)))))
    },
};