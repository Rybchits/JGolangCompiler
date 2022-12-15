#pragma once

#include "./java_entity.h"
#include "./java_entity_visitor.h"
#include "../ast.h"

#include <vector>
#include <unordered_map>
#include <iostream>

class Semantic {
private:
    static const std::string GlobalClassName;
    static bool isGeneratedName(const std::string_view name);

    PackageAST* root;

    std::unordered_map<std::string, JavaClass> classes = { {GlobalClassName,JavaClass(nullptr)} };
    std::vector<JavaVariable> globals;

    void analyzePackageScope();
    void findAnonymousClass();

public:
    explicit Semantic(PackageAST* package): root(package) {};

    bool analyze();
    std::vector<std::string> errors;
};
