#pragma once

#include <vector>
#include "java_method.h"
#include "../ast.h"

class JavaMethod;

class JavaClass {
private:


    struct JavaClassField {
        std::string id;
        TypeAST* type;
    };

    std::vector<JavaClassField> fields;
    std::vector<JavaMethod> methods;

    std::vector<std::string> interfaces;

};
