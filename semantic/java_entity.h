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
    
    bool equals(const JavaArraySignature* other);
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
        UserType,
        Invalid
    } type;

    std::variant<std::string, JavaArraySignature*> value;
    
    JavaType(): type(Invalid) {};
    JavaType(TypeAST* node);
    JavaType(JavaArraySignature* array): type(Array), value(array) {};
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

    BlockStatement* block;  // Содержится в узле functionNode. Мб излишне

public:
    JavaFunction() = default;
    JavaFunction(FunctionDeclaration* node);

    //std::vector<std::pair<, std::string>> toRowsConstantTable();
};


class JavaClass {
    std::unordered_map<std::string, JavaType*> fields;
    std::unordered_map<std::string, JavaFunction*> methods;

    std::unordered_map<size_t, std::string> constantTable;

public:
    void addMethod(std::string identifier, JavaFunction * method) { methods.emplace(identifier, method); }
};