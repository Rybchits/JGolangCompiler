#pragma once

#include "../visitor.h"
#include "../semantic/entities.h"
#include "./constant.h"

#include <unordered_map>

class Generator {
public:
    void generate(std::unordered_map<std::string, ClassEntity*> & classPool);
    //void generate(Constant constant);
    //void generate(MethodEntity* method);
};
