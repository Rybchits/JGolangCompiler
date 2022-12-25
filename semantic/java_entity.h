#pragma once

#include "../ast.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

class JavaType;
class JavaVariable;
class JavaFunction;

class JavaArraySignature {
public:
    int dims;
    JavaType* type;
    JavaArraySignature(int dims, JavaType* javaType): dims(dims), type(javaType) {};
    JavaArraySignature(JavaType* javaType): dims(-1), type(javaType) {};
    
    bool equals(const JavaArraySignature* other) const;
};


class JavaFunctionSignature {
public:
    JavaType* returnType;
    std::list<JavaType*> argsTypes;

    JavaFunctionSignature(std::list<JavaType*>& args, JavaType* returnType): argsTypes(args), returnType(returnType) {};
    bool equals(const JavaFunctionSignature* other) const;
};


class JavaType {
public:
    enum JavaTypeEnum {
        Int,
        UntypedInt,
        Float,
        UntypedFloat,
        Boolean,
        Rune,
        String,
        Array,
        Function,
        UserType,
        Invalid
    } type;

    std::variant<std::string, JavaArraySignature*, JavaFunctionSignature*> value;
    
    JavaType(): type(Invalid) {};
    JavaType(TypeAST* node);
    JavaType(JavaArraySignature* array): type(Array), value(array) {};
    JavaType(JavaFunctionSignature* function): type(Function), value(function) {};
    JavaType(JavaTypeEnum type): type(type) {};
 
    std::string toByteCode() const;
    bool isNumeric();

    bool equal(const JavaType* other);
    JavaType* determinePriorityType(const JavaType* other);

    static bool IsBuiltInType(std::string);
    static std::list<std::string> BuiltInTypes;

private:
    JavaTypeEnum builtInTypeFromString(std::string id);
};


class JavaFunction {
private:
    int numberLocalVariables = 0;

    std::unordered_map<std::string, JavaType*> arguments;
    JavaType* returnType;

    BlockStatement* block;

public:
    JavaFunction() {};
    JavaFunction(FunctionDeclaration* node);

    JavaType* toJavaType();
    
    const std::unordered_map<std::string, JavaType*> getArguments() { return arguments; }
    //std::vector<std::pair<, std::string>> toRowsConstantTable();
};


class JavaClass {
private:
    std::unordered_map<std::string, JavaType*> fields;
    std::unordered_map<std::string, JavaFunction*> methods;

    std::unordered_map<size_t, std::string> constantTable;

public:
    void addMethod(std::string identifier, JavaFunction * method) { methods.emplace(identifier, method); }
    void addField(std::string identifier, JavaType* type) { fields.emplace(identifier, type); };
    
    const std::unordered_map<std::string, JavaFunction*> getMethods() { return methods; }; 
};