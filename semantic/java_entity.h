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
        Void,
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
    bool isInteger();
    bool isFloat();

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
    BlockStatement* getCodeBlock() { return block; }
    
    void setNumberLocalVariables(int number);
    int getNumberLocalVariables() const;
};


class JavaClass {
private:
    std::unordered_map<std::string, JavaType*> fields;
    std::unordered_map<std::string, JavaFunction*> methods;

public:
    bool addMethod(std::string identifier, JavaFunction * method) { return methods.try_emplace(identifier, method).second; }
    bool addField(std::string identifier, JavaType* type) { return fields.try_emplace(identifier, type).second; };
    bool addFields(std::unordered_map<std::string, JavaType*> & vars);
    
    const std::unordered_map<std::string, JavaFunction*>& getMethods() { return methods; }; 
    const std::unordered_map<std::string, JavaType*>& getFields() { return fields; };

};