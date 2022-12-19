#pragma once

#include "../ast.h"

#include <string>
#include <vector>
#include <variant>

class JavaType;
class JavaVariable;
class JavaMethod;
class JavaClass;

struct JavaArraySignature {
    int dims;
    JavaType* type;

    JavaArraySignature(int dims, JavaType* javaType): dims(dims), type(javaType) {};
};

class JavaType {
public:
    enum class BuiltInType {
        Int,
        Float,
        Boolean,
        String,
        Array,
        UserType,
    } type;

    JavaType(std::string typeAsId);
    JavaType(JavaArraySignature array);

    std::variant<std::string, JavaArraySignature> value;

    std::string toByteCode() const;
};

class JavaVariable {
private:
    std::string id;
    JavaType* type;

public:
    JavaVariable(std::string id, JavaType* type): id(id), type(type) {};
};


class JavaMethod {
private:
    FunctionDeclaration* functionNode;

    std::string receiverIdentifier;
    JavaType* receiverType;

    std::vector<JavaVariable> args;
    JavaType* returnType;
    BlockStatement* block;

public:
    JavaMethod(FunctionDeclaration* node): functionNode(node) {};

    JavaMethod(FunctionDeclaration* node, std::string thisId, JavaType* className):
        functionNode(node), receiverIdentifier(thisId), receiverType(className) {};
};


class JavaClass {
public:
    enum class TypeJavaClass {
        Class,
        Interface,
        Alias,
    };

private:
    TypeAST* typeNode;
    TypeJavaClass typeJavaClass;

    std::vector<JavaVariable> fields;
    std::vector<JavaMethod> methods;

public:
    JavaClass() = default;
    JavaClass(TypeAST* node);

    void addMethod(JavaMethod&& method);
    void addField(JavaVariable&& field);
    void addField(const JavaVariable& field);
    void addField(std::vector<JavaVariable>&& newFields);
    void addField(const std::vector<JavaVariable>& field);

    void constructFields();
};
