#pragma once

#include "../visitor.h"
#include "../semantic/entities.h"
#include "./constant.h"

#include <unordered_map>

class Generator {
private:
    //void generate(std::string, ClassEntity*);
    //void generate(Constant constant);
    //void generate(MethodEntity* method);
public:
    void generate(std::unordered_map<std::string, ClassEntity*> & classPool);
    void generate(std::ostream& out, Constant & constant);
    ConstantPool fillConstantPool(std::string className, ClassEntity* classEntity);
};
