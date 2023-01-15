#pragma once

#include "../ast.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

class TypeEntity;
class VariableEntity;
class MethodEntity;

class ArraySignatureEntity {
public:
    int dims;
    TypeEntity* elementType;
    ArraySignatureEntity(int dims, TypeEntity* TypeEntity): dims(dims), elementType(TypeEntity) {};
    ArraySignatureEntity(TypeEntity* TypeEntity): dims(-1), elementType(TypeEntity) {};
    
    bool equal(const ArraySignatureEntity* other) const;
    bool isSlice() const;
    TypeEntity* typeAxis(int indexAxis);
};


class FunctionSignatureEntity {
public:
    TypeEntity* returnType;
    std::list<TypeEntity*> argsTypes;

    FunctionSignatureEntity(std::list<TypeEntity*>& args, TypeEntity* returnType): argsTypes(args), returnType(returnType) {};
    bool equals(const FunctionSignatureEntity* other) const;
};


class TypeEntity {
public:
    enum TypeEntityEnum {
        Void,
        Int,
        UntypedInt,
        Float,
        UntypedFloat,
        Boolean,
        String,
        Array,
        Function,
        BuiltInFunction,
        UserType,
        Invalid
    } type;

    std::variant<std::string, ArraySignatureEntity*, FunctionSignatureEntity*> value;
    
    TypeEntity(): type(Invalid) {};
    TypeEntity(TypeAST* node);
    TypeEntity(ArraySignatureEntity* array): type(Array), value(array) {};
    TypeEntity(FunctionSignatureEntity* function): type(Function), value(function) {};
    TypeEntity(TypeEntityEnum type): type(type) {};
 
    std::string toByteCode() const;
    bool isNumeric();
    bool isInteger();
    bool isFloat();

    bool equal(const TypeEntity* other);
    TypeEntity* determinePriorityType(const TypeEntity* other);

    static bool IsBuiltInType(std::string);
    static std::list<std::string> BuiltInTypes;

private:
    TypeEntityEnum builtInTypeFromString(std::string id);
};


class MethodEntity {
private:
    int numberLocalVariables = 0;

    std::vector<std::pair<std::string, TypeEntity*>> arguments;
    TypeEntity* returnType;

    BlockStatement* block;

public:
    MethodEntity() {};
    MethodEntity(FunctionDeclaration* node);

    TypeEntity* toTypeEntity();
    
    const std::vector<std::pair<std::string, TypeEntity*>> & getArguments() { return arguments; }
    BlockStatement* getCodeBlock() const { return block; }
    int getNumberLocalVariables() const;
    TypeEntity* getReturnType() const { return returnType; };
    
    void setNumberLocalVariables(int number);
};


class FieldEntity {
public:
    TypeEntity * type;
    ExpressionAST * declaration;
    FieldEntity(TypeEntity * type, ExpressionAST * declaration) : type(type), declaration(declaration) {};
    
    bool hasDeclaration() { return declaration == nullptr ? false : true; }
};


class ClassEntity {
private:
    std::unordered_map<std::string, FieldEntity*> fields;
    std::unordered_map<std::string, MethodEntity*> methods;

public:
    bool addMethod(std::string identifier, MethodEntity * method) { return methods.try_emplace(identifier, method).second; }
    bool addField(std::string identifier, FieldEntity* type) { return fields.try_emplace(identifier, type).second; };
    bool addFields(std::unordered_map<std::string, FieldEntity*> & vars);
    
    const std::unordered_map<std::string, MethodEntity*>& getMethods() { return methods; };
    const std::unordered_map<std::string, FieldEntity*>& getFields() { return fields; };
    
    bool hasFieldsDeclaration() { 
        for (auto & [_, field] : fields) {if (field->hasDeclaration()) return true; };
                                                                       return false;}
};


class VariableEntity{
public:
    TypeEntity* type;
    bool isConst;
    bool isArgument;
    int numberUsage = 0;

    VariableEntity(TypeEntity* type, bool isConst = false, bool isArgument = false): 
        type(type), isConst(isConst), isArgument(isArgument) {};

    void use() { numberUsage++; };
};