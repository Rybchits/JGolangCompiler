#pragma once

#include "../ast.h"
#include "./type_entity.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>
#include <memory>

class MethodEntity {
private:
    int numberLocalVariables = 0;

    std::vector<std::pair<std::string, TypePtr>> arguments;
    TypePtr returnType;

    BlockStatement* block;

public:
    MethodEntity() {};
    MethodEntity(FunctionDeclaration* node);

    TypePtr toTypeEntity() const;
    
    const std::vector<std::pair<std::string, TypePtr>> & getArguments() const { return arguments; }
    BlockStatement* getCodeBlock() const { return block; }
    int getNumberLocalVariables() const;
    TypePtr getReturnType() const { return returnType; };
    
    void setNumberLocalVariables(int number);
};


class FieldEntity {
public:
    TypePtr type;
    ExpressionAST * declaration;
    FieldEntity(TypePtr type, ExpressionAST * declaration) : type(std::move(type)), declaration(declaration) {};
    
    bool hasDeclaration() { return declaration == nullptr ? false : true; }
};


class ClassEntity {
private:
    std::unordered_map<std::string, std::unique_ptr<FieldEntity>> fields;
    std::unordered_map<std::string, std::unique_ptr<MethodEntity>> methods;

public:
    bool addMethod(std::string identifier, std::unique_ptr<MethodEntity> method) {
        return methods.try_emplace(std::move(identifier), std::move(method)).second;
    }
    bool addField(std::string identifier, std::unique_ptr<FieldEntity> field) {
        return fields.try_emplace(std::move(identifier), std::move(field)).second;
    }
    bool addFields(std::unordered_map<std::string, std::unique_ptr<FieldEntity>> vars);
    
    const std::unordered_map<std::string, std::unique_ptr<MethodEntity>>& getMethods() const { return methods; };
    const std::unordered_map<std::string, std::unique_ptr<FieldEntity>>& getFields() const { return fields; };
    
    bool hasFieldsDeclaration() const {
        for (auto & [_, field] : fields) {if (field->hasDeclaration()) return true; };
                                                                       return false;}
};
