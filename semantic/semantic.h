#pragma once

#include "./entities.h"
#include "../ast.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string_view>

class Semantic {
    friend class TypesVisitor;
private:
    Semantic(PackageAST* package): root(package) {};
    static Semantic* instance;

    Semantic(Semantic &other) = delete;
    void operator=(const Semantic &) = delete;

    PackageAST* root;

    std::list<FunctionDeclaration*> packageFunctions;
    std::list<VariableDeclaration*> packageVariables;
    
    void analyzePackageScope();
    void transformRoot();
    void transformExpressions();
    void createGlobalClass();

    static const std::string GlobalClassName;
    bool isGeneratedName(const std::string_view name);
    void printErrors();

public:
    static Semantic *GetInstance(PackageAST* package);

    ClassEntity* packageClass;
    std::unordered_map<size_t, TypeEntity*> typesExpressions;

    static const std::vector<std::string> BuiltInFunctions;
    static bool IsBuiltInFunction(std::string identifier);

    bool analyze();
    std::vector<std::string> errors;
};
