#pragma once

#include "../ast.h"
#include "./type_entity.h"

#include <unordered_map>
#include <string>
#include <vector>
#include <variant>

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