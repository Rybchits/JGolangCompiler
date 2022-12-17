#pragma once

#include "./java_entity.h"
#include "./java_entity_visitor.h"
#include "../ast.h"

#include <vector>
#include <unordered_map>
#include <iostream>

class Semantic {
private:
    Semantic(PackageAST* package): root(package) {};
    static Semantic* instance;

    Semantic(Semantic &other) = delete;
    void operator=(const Semantic &) = delete;

    static const std::string GlobalClassName;
    bool isGeneratedName(const std::string_view name);

    PackageAST* root;
    std::unordered_map<std::string, JavaClass> classes = { {GlobalClassName, JavaClass(nullptr)} };
    std::vector<JavaVariable> globals;

    void analyzePackageScope();
    void transformRoot();

public:
    static Semantic *GetInstance(PackageAST* package);

    bool analyze();
    std::vector<std::string> errors;
};
