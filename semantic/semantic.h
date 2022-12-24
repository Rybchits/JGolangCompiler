#pragma once

#include "./java_entity.h"
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
    std::unordered_map<std::string, JavaClass*> classes = { { GlobalClassName, new JavaClass()} };

    void analyzePackageScope();
    void transformRoot();
    void analyzeTypesAndVariables();

    static const std::string GlobalClassName;
    bool isGeneratedName(const std::string_view name);

public:
    static Semantic *GetInstance(PackageAST* package);

    bool analyze();
    std::vector<std::string> errors;
};
