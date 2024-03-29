#pragma once

#include "../entities/class_entity.h"
#include "../entities/type_entity.h"

#include "../ast.h"
#include "./types_visitor.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string_view>

class TypesVisitor;

class Semantic {
private:
    PackageAST* root;

    std::list<FunctionDeclaration*> packageFunctions;
    std::list<VariableDeclaration*> packageVariables;

    std::vector<std::string> errors;
    
    TypesVisitor* typeVisitor;
    
    void analyzePackageScope();
    void transformStatements();
    void precalculateExpressions();
    
    void createPackageClass();

    bool isGeneratedName(const std::string_view name);
    void printErrors();

public:
    Semantic(PackageAST* package);

    ClassEntity* packageClass;
    TypesVisitor* getTypesVisitor();

    void addError(std::string message);

    static const std::vector<std::string> BuiltInFunctions;
    static bool IsBuiltInFunction(std::string identifier);

    bool analyze();
};
