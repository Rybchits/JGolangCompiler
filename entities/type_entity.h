#pragma once

#include <string>
#include <list>
#include <variant>

#include "../ast.h"

class TypeAST;
class TypeEntity;

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
        Any,                // for blank variables
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
    TypeEntity(TypeEntityEnum type, std::string id = ""): type(type), value(id) {};
 
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