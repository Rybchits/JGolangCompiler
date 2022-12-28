#include "codegen.h"
#include "constant.h"

#include <filesystem>
#include <fstream>
#include <iostream>

std::vector<char> intToBytes(int value) {
	std::vector<char> arrayOfByte(4);
	for (int i = 0; i < 4; ++i) {
		arrayOfByte[3 - i] = (value >> (i * 8));
	}
	return arrayOfByte;
}


std::vector<char> floatToBytes(float value)
{
	std::vector<char> arrayOfByte(4);

	for (int i = 0; i < sizeof(float); ++i)
		arrayOfByte[3 - i] = ((char*)&value)[i];
	return arrayOfByte;
}


ConstantPool Generator::fillConstantPool(std::string className, ClassEntity* classEntity) {
    ConstantPool constantPool;

    // Add name of "Code" attribute
    constantPool.pool.push_back(Constant::CreateUtf8("Code"));
    
    // Add class name and class
    //constantPool.pool.push_back(Constant::CreateUtf8());
    constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8(className)));

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

    return constantPool;
}

void Generator::generate(std::unordered_map<std::string, ClassEntity*> & classes) {
    using namespace std::filesystem;

    for (auto & [className, classEntity] : classes) {
        ConstantPool pool = fillConstantPool(className, classEntity);

        const auto filename = std::string{ className } + ".class";

        auto filepath = current_path() / "output" / filename;
        create_directory(current_path() / "output");

        std::fstream out;
        out.open(filepath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        std::vector<char> len = intToBytes(1);

        out << (char)0xCA << (char)0xFE << (char)0xBA << (char)0xBE;

        out << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x34;

        std::cout << len[2] << len[3];

        // Interfaces table
		std::cout << (char)0x00 << (char)0x00;
		std::cout << (char)0x00 << (char)0x00;

    }
}
