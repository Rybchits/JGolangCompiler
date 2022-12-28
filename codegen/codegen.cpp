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

void Generator::generate(std::ostream& out, Constant & constant) {
	// UTF-8
	if (constant.type == Constant::TypeT::Utf8) {
		char const* c = constant.utf8.c_str();
	    out << (char)Constant::TypeT::Utf8;
		std::vector<char> len = intToBytes(strlen(c));
	    out << (char)len[2] << (char)len[3];
		for (int i = 0; i < strlen(c); ++i) {
			out << c[i];
		}
	}

	// Integer
	if (constant.type == Constant::TypeT::Integer) {
		out << (char)Constant::TypeT::Integer;
		std::vector<char> len = intToBytes(constant.integer);
		out << len[0] << len[1] << len[2] << len[3];
	}

	// Float
	if (constant.type == Constant::TypeT::Float) {
		out << (char)Constant::TypeT::Float;
		std::vector<char> len = floatToBytes(constant.floating);
		out << len[0] << len[1] << len[2] << len[3];
	}

	// Class
	if (constant.type == Constant::TypeT::Class) {
		out << (char)Constant::TypeT::Class;
		std::vector<char> len = intToBytes(constant.classNameId);
		out << len[2] << len[3];
	}

	// String
	if (constant.type == Constant::TypeT::String) {
		out << (char)Constant::TypeT::String;
		std::vector<char> len = intToBytes(constant.utf8Id);
		out << len[2] << len[3];
	}

	// Fieldref
	if (constant.type == Constant::TypeT::FieldRef) {
		out << (char)Constant::TypeT::FieldRef;
		std::vector<char> len = intToBytes(constant.classId);
		out << len[2] << len[3];
		len = intToBytes(constant.nameAndTypeId);
		out << len[2] << len[3];
	}

	// Methodref
	if (constant.type == Constant::TypeT::MethodRef) {
		out << (char)Constant::TypeT::MethodRef;
		std::vector<char> len = intToBytes(constant.classId);
		out << len[2] << len[3];
		len = intToBytes(constant.nameAndTypeId);
		out << len[2] << len[3];
	}

	// NameAndTypeT
	if (constant.type == Constant::TypeT::NameAndType) {
		out << (char)Constant::TypeT::NameAndType;
		std::vector<char> len = intToBytes(constant.nameAndTypeId);
		out << len[2] << len[3];
		len = intToBytes(constant.typeId);
		out << len[2] << len[3];
	}
}


ConstantPool Generator::fillConstantPool(std::string className, ClassEntity* classEntity) {
    ConstantPool constantPool;

    // constantPool.pool.push_back(Constant::CreateUtf8("this"));

    // Add name of "Code" attribute
    // constantPool.pool.push_back(Constant::CreateUtf8("Code"));
    
    // Current class
    constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8(className)));
	constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8("java/lang/Object")));

    // constantPool.FindMethodRef("java/lang/Object", "<init>", "()V");
     
    // Add name of name, name of types and N&T of fields
    // for (auto & [fieldIdentifier, field] : classEntity->getFields()) {
        //constantPool.FindFieldRef(className, fieldIdentifier, field->type->toByteCode());
    // }

    // for (auto & [methodIdentifier, method] : classEntity->getMethods()) {
        //constantPool.FindMethodRef(className, methodIdentifier, method->toTypeEntity()->toByteCode());
    // }

    // constantPool.pool.push_back(Constant::CreateUtf8(std::string("L") + className + ";"));

    return constantPool;
}

void Generator::generate(std::unordered_map<std::string, ClassEntity*> & classes) {
    using namespace std::filesystem;

    for (auto & [className, classEntity] : classes) {
        ConstantPool pool = fillConstantPool(className, classEntity);

        // Create class file
        const auto filename = std::string{ className } + ".class";

        auto filepath = current_path() / "output" / filename;
        create_directory(current_path() / "output");

        std::fstream out;
        out.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

		// CAFEBABE
		out << (char)0xCA << (char)0xFE << (char)0xBA << (char)0xBE;

		// JAVA 8 (version 52.0 (0x34))
		out << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x34;

		// constants count
		std::vector<char> sizeConstantPool = intToBytes(pool.pool.size() + 1);
		out << sizeConstantPool[2] << sizeConstantPool[3];

		//constants table
        for (auto & constant : pool.pool) {
            generate(out, constant);
        }

		// Flags 
		out << (char)0x00 << (char)0x21;

		// This class constant
		std::vector<char> bytes = intToBytes(pool.FindClass(className));
		out << bytes[2] << bytes[3];

		// Parent class constant
		bytes = intToBytes(pool.FindClass("java/lang/Object"));
		out << bytes[2] << bytes[3];

		// Interfaces table
		out << (char)0x00 << (char)0x00;

		// fields count
		out << (char)0x00 << (char)0x00;

		// methods count
		bytes = intToBytes(0);
		out << bytes[2] << bytes[3];

        // atributes
		out << (char)0x00 << (char)0x00;
    }
}
