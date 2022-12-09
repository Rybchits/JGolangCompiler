#pragma once
#include <vector>

#include "../ast.h"
#include "java_class.h"
#include "java_method.h"
#include "java_interface.h"

class Semantic {
private:
    PackageAST* root;

    // Declared global
    std::vector<JavaClass> classes;
    std::vector<JavaMethod> functions;
    std::vector<JavaInterface> interfaces;

    std::vector<std::string> errors;

public:
    explicit Semantic(PackageAST* package): root(package) {};

    void analyze();

    void analyzeGlobalClasses();
};
