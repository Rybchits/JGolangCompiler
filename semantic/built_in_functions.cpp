#include <unordered_map>
#include "semantic.h"

const std::unordered_map<std::string, JavaType*> Semantic::BuiltInFunctions = {
    {"print", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::Void)))
    },

    {"println", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::Void)))
    },

    {"len", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(new JavaArraySignature(new JavaType(JavaType::Any)))})),
        new JavaType(JavaType::Int)))
    },

    {"scan", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::Void)))
    },

    {"append", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({
            new JavaType(new JavaArraySignature(new JavaType(JavaType::Any))),
            new JavaType(JavaType::Any)
        })),
        new JavaType(new JavaArraySignature(new JavaType(JavaType::Any)))))
    },

    // Conversions
    {"int", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::Int)))
    },

    {"float32", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::Float)))
    },

    {"float64", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::Float)))
    },

    {"string", new JavaType(new JavaFunctionSignature(
        *(new std::list<JavaType*>({new JavaType(JavaType::Any)})),
        new JavaType(JavaType::String)))
    },
};