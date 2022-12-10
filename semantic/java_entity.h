#pragma once
#include <string>
#include <vector>

class JavaType {
    enum BuiltInType {
        Byte,
        Short,
        Int,
        Long,
        Float,
        Double,
        Boolean,
        String,
        Array
    };

};

class JavaVariable {
    
};

class JavaMethod {
    std::vector<JavaVariable> local_variables;      //args + local variables
};

class JavaClass {
private:
    struct JavaClassField {
        std::string id;
        JavaType* type;
    };

    std::vector<JavaClassField> fields;
    std::vector<JavaMethod> methods;

    std::vector<std::string> interfaces;
};
