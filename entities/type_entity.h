#pragma once

#include <string>
#include <list>
#include <variant>
#include <memory>
#include <utility>

class TypeAST;
class TypeEntity;

using TypePtr = std::shared_ptr<const TypeEntity>;

class ArraySignatureEntity {
public:
    int dims;
    TypePtr elementType;
    ArraySignatureEntity(int dims, TypePtr elementType): dims(dims), elementType(std::move(elementType)) {};
    explicit ArraySignatureEntity(TypePtr elementType): dims(-1), elementType(std::move(elementType)) {};
    
    bool equal(const ArraySignatureEntity& other) const;
    bool isSlice() const;
};


class FunctionSignatureEntity {
public:
    TypePtr returnType;
    std::list<TypePtr> argsTypes;

    FunctionSignatureEntity(std::list<TypePtr> args, TypePtr returnType): returnType(std::move(returnType)), argsTypes(std::move(args)) {};
    bool equals(const FunctionSignatureEntity& other) const;
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

    std::variant<std::string, ArraySignatureEntity, FunctionSignatureEntity> value;
    
    TypeEntity(): type(Invalid) {};
    explicit TypeEntity(const TypeAST* node);
    explicit TypeEntity(ArraySignatureEntity array): type(Array), value(std::move(array)) {};
    explicit TypeEntity(FunctionSignatureEntity function): type(Function), value(std::move(function)) {};
    TypeEntity(TypeEntityEnum type, std::string id = ""): type(type), value(id) {};
 
    std::string toByteCode() const;
    bool isNumeric() const;
    bool isInteger() const;
    bool isFloat() const;

    bool equal(const TypePtr& other) const;

    static bool IsBuiltInType(std::string);
    static std::list<std::string> BuiltInTypes;

private:
    TypeEntityEnum builtInTypeFromString(std::string id);
};

TypePtr determinePriorityType(const TypePtr& lhs, const TypePtr& rhs);
TypePtr typeAxis(TypePtr type, int indexAxis);
