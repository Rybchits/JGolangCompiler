#include "codegen.h"

void Generator::generate(std::unordered_map<std::string, ClassEntity*> & classPool) {
    for (auto & [className, classEntity] : classPool) {
        ClassFile ClassEntityFile;

        ConstantPool constantPool;

        // Add name of "Code" attribute
        constantPool.pool.push_back(Constant::CreateUtf8("Code"));
        // Add class name and class
        constantPool.pool.push_back(Constant::CreateUtf8(className));
        constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8(className)));

        {
        // Add default constructor name utf8
        constantPool.pool.push_back(Constant::CreateUtf8("<init>"));
        IdT nameId = constantPool.pool.size();
        // Add default constructor type utf8
        constantPool.pool.push_back(Constant::CreateUtf8("()V"));
        IdT typeId = constantPool.pool.size();

        if (!classEntity->getFields().empty()) {
            constantPool.pool.push_back(Constant::CreateNaT(nameId, typeId));
            IdT NaTId = constantPool.pool.size();
            // Add methodRef
            constantPool.pool.push_back(Constant::CreateFieldRef(NaTId, constantPool.FindClass(className)));
        }
        }

        // Add name of name, name of types and N&T of fields
        for (auto & [fieldIdentifier, field] : classEntity->getFields()) {
            // Adding of fieldIdentifier utf8 inside FindUtf8()
            constantPool.pool.push_back(Constant::CreateUtf8(fieldIdentifier));
            IdT nameId = constantPool.pool.size();
            // Add type utf8
            IdT typeId = constantPool.FindUtf8(field->type->toByteCode());
            // Add N&T
            constantPool.pool.push_back(Constant::CreateNaT(nameId, typeId));
            IdT NaTId = constantPool.pool.size();
            // Add FieldRef
            constantPool.pool.push_back(Constant::CreateFieldRef(NaTId, constantPool.FindClass(className)));

        }

        for (auto & [methodIdentifier, method] : classEntity->getMethods()) {
            // Adding of method utf8 inside FindUtf8()
            constantPool.pool.push_back(Constant::CreateUtf8(methodIdentifier));
            IdT nameId = constantPool.pool.size();
            // Add type utf8
            IdT typeId = constantPool.FindUtf8(method->toTypeEntity()->toByteCode());
            // Add N&T
            constantPool.pool.push_back(Constant::CreateNaT(nameId, typeId));
            IdT NaTId = constantPool.pool.size();
            // Add methodRef
            constantPool.pool.push_back(Constant::CreateFieldRef(NaTId, constantPool.FindClass(className)));
        }
    }
}