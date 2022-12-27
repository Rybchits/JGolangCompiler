#include "codegen.h"

void Generator::generate(std::unordered_map<std::string, JavaClass*> & classPool) {
    for(auto & [className, javaClass] : classPool) {
        ClassFile javaClassFile;

        ConstantPool constantPool;

        // Add name of "Code" attribute
        constantPool.pool.push_back(Constant::CreateUtf8("Code"));
        // Add class name and class
        constantPool.pool.push_back(Constant::CreateUtf8(className));
        constantPool.pool.push_back(Constant::CreateClass(constantPool.FindClass(className)));

        // Add name of name, name of types and N&T of fields
        for (auto & [fieldIdentifier, fieldType] : javaClass->getFields()) {
            // Adding of fieldType, fieldIdentifier utf8 inside FindUtf8()
            constantPool.pool.push_back(Constant::CreateUtf8(fieldIdentifier));
            IdT nameId = constantPool.pool.size() - 1;
            // Add type utf8
            IdT typeId = constantPool.FindUtf8(fieldType->toByteCode());
            // Add N&T
            constantPool.pool.push_back(Constant::CreateNaT(nameId, typeId));
            IdT NaTId = constantPool.pool.size() - 1;
            // Add FieldRef
            constantPool.pool.push_back(Constant::CreateFieldRef(NaTId, constantPool.FindClass(className)));
        }

        // Add default constructor name utf8
        constantPool.pool.push_back(Constant::CreateUtf8("<init>"));
        IdT nameId = constantPool.pool.size() - 1;
        // Add default constructor type utf8
        constantPool.pool.push_back(Constant::CreateUtf8("<()V>"));
        IdT typeId = constantPool.pool.size() - 1;
        // Add default constructor N&T
        constantPool.pool.push_back(Constant::CreateNaT(nameId, typeId));
        IdT NaTId = constantPool.pool.size() - 1;
        // Add default constructor method ref
        constantPool.pool.push_back(Constant::CreateMethodRef(NaTId, constantPool.FindClass(className)));

        for (auto & [methodIdentifier, method] : javaClass->getMethods()) {
            constantPool.pool.push_back(Constant::CreateUtf8(methodIdentifier));
            IdT nameId = constantPool.pool.size() - 1;
            // Adding of method utf8 inside FindUtf8()
            constantPool.FindUtf8(method->toJavaType()->toByteCode());
            // Add type utf8
            IdT typeId = constantPool.FindUtf8(fieldType->toByteCode());
            // Add N&T
            constantPool.pool.push_back(Constant::CreateNaT(nameId, typeId));
            IdT NaTId = constantPool.pool.size() - 1;
            // Add methodRef
            constantPool.pool.push_back(Constant::CreateFieldRef(NaTId, constantPool.FindClass(className)));
        }
    }
}