#pragma once

#include "../ast.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

class JavaType;
class JavaVariable;
class JavaFunction;

struct JavaArraySignature {
    int dims;
    JavaType* type;

    JavaArraySignature(int dims, JavaType* javaType): dims(dims), type(javaType) {};
};

class JavaType {
public:
    enum BuiltInType {
        Int,
        UndefinedInt,
        Float,
        UndefinedFloat,
        Boolean,
        String,
        Array,
        UserType,
        Unknown,
    } type;
    
    JavaType(): type(BuiltInType::Unknown) {};
    JavaType(TypeAST* node);
    JavaType(std::string id);
    std::variant<std::string, JavaArraySignature> value;
 
    std::string toByteCode() const;

    bool operator==(const JavaType& other);

private:
    BuiltInType builtInTypeFromString(std::string id);
};


class JavaFunction {
private:
    FunctionDeclaration* functionNode;

    int numberLocalVariables = 0;
    JavaType* returnType;
    BlockStatement* block;

public:
    JavaFunction() = default;
    JavaFunction(FunctionDeclaration* node): functionNode(node) {};
};


class JavaClass {
    std::unordered_map<std::string, JavaType> fields;
    std::unordered_map<std::string, JavaFunction> methods;

    std::unordered_map<size_t, std::string> constantTable;
};