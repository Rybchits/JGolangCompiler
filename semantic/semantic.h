#pragma once

#include "./entities.h"
#include "../ast.h"

#include <vector>
#include <unordered_map>
#include <iostream>
#include <string_view>

class Semantic {
    friend class TypeCheckVisitor;
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
    void createGlobalClass();

    static const std::string GlobalClassName;
    bool isGeneratedName(const std::string_view name);

public:
    static Semantic *GetInstance(PackageAST* package);
    static const std::unordered_map<std::string, TypeEntity*> BuiltInFunctions;
    ClassEntity* packageClass;

    bool analyze();
    std::vector<std::string> errors;
};
