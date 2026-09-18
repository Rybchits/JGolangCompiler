#pragma once

#include "../entities/class_entity.h"
#include "../entities/type_entity.h"

#include "../ast.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string_view>
#include <memory>

class Semantic {
private:
    PackageAST* root;

    std::list<FunctionDeclaration*> packageFunctions;
    std::list<VariableDeclaration*> packageVariables;

    std::vector<std::string> errors;
    
    void analyzePackageScope();
    void transformStatements();
    void precalculateExpressions();
    
    void createPackageClass();

    bool isGeneratedName(const std::string_view name);
    void printErrors();

public:
    Semantic(PackageAST* package);

    std::unique_ptr<ClassEntity> packageClass;

    bool analyze();
};
