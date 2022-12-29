#pragma once

#include "../visitor.h"
#include "../semantic/entities.h"
#include "./constant.h"
#include "./constant_visitor.h"

#include <unordered_map>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

class Generator {
private:
    void generate(std::ostream& out, Constant & constant);
    void generate(std::ostream& out, std::string methodName, MethodEntity* method, ConstantPool* pool);
    void generate(std::ostream& out, std::string fieldName, FieldEntity* field, ConstantPool* pool);
    void generateGlobalClassConstructor(std::ostream& out, const std::string className, ConstantPool* pool);
    //void generateStaticConstructor();

    ConstantPool fillConstantPool(std::string className, ClassEntity* classEntity);
public:
    void generate(std::unordered_map<std::string, ClassEntity*> & classPool);
};
