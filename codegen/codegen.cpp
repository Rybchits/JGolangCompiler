#include "codegen.h"


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

void Generator::generateConstant(Constant & constant) {
	// UTF-8
	if (constant.type == Constant::TypeT::Utf8) {
		char const* c = constant.utf8.c_str();
	    outfile << (char)Constant::TypeT::Utf8;
		std::vector<char> len = intToBytes(strlen(c));
	    outfile << (char)len[2] << (char)len[3];
		for (int i = 0; i < strlen(c); ++i) {
			outfile << c[i];
		}
	}

	// Integer
	if (constant.type == Constant::TypeT::Integer) {
		outfile << (char)Constant::TypeT::Integer;
		std::vector<char> len = intToBytes(constant.integer);
		outfile << len[0] << len[1] << len[2] << len[3];
	}

	// Float
	if (constant.type == Constant::TypeT::Float) {
		outfile << (char)Constant::TypeT::Float;
		std::vector<char> len = floatToBytes(constant.floating);
		outfile << len[0] << len[1] << len[2] << len[3];
	}

	// Class
	if (constant.type == Constant::TypeT::Class) {
		outfile << (char)Constant::TypeT::Class;
		std::vector<char> len = intToBytes(constant.classNameId);
		outfile << len[2] << len[3];
	}

	// String
	if (constant.type == Constant::TypeT::String) {
		outfile << (char)Constant::TypeT::String;
		std::vector<char> len = intToBytes(constant.utf8Id);
		outfile << len[2] << len[3];
	}

	// Fieldref
	if (constant.type == Constant::TypeT::FieldRef) {
		outfile << (char)Constant::TypeT::FieldRef;
		std::vector<char> len = intToBytes(constant.classId);
		outfile << len[2] << len[3];
		len = intToBytes(constant.nameAndTypeId);
		outfile << len[2] << len[3];
	}

	// Methodref
	if (constant.type == Constant::TypeT::MethodRef) {
		outfile << (char)Constant::TypeT::MethodRef;
		std::vector<char> len = intToBytes(constant.classId);
		outfile << len[2] << len[3];
		len = intToBytes(constant.nameAndTypeId);
		outfile << len[2] << len[3];
	}

	// NameAndTypeT
	if (constant.type == Constant::TypeT::NameAndType) {
		outfile << (char)Constant::TypeT::NameAndType;
		std::vector<char> len = intToBytes(constant.nameId);
		outfile << len[2] << len[3];
		len = intToBytes(constant.typeId);
		outfile << len[2] << len[3];
	}
}


void Generator::addBuiltInFunctions(std::string nameBaseClass, std::unordered_map<std::string, TypeEntity*> functions) {
	for (auto &[id, descriptor] : functions) {
		int index = constantPool.FindMethodRef(nameBaseClass, id, descriptor->toByteCode());
		context.addConstant(id, new RefConstant(index, false));
	}
}


void Generator::fillConstantPool(std::string className, ClassEntity* classEntity) {
	auto constantVisitor = new ConstantVisitor(&constantPool);
	
    // Add name of "Code" attribute
    constantPool.pool.push_back(Constant::CreateUtf8("Code"));
    
    // Current class
    constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8(className)));
	constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8("java/lang/Object")));

    constantPool.FindMethodRef("java/lang/Object", "<init>", "()V");
     
    // Add name of name, name of types and N&T of fields
    for (auto & [fieldIdentifier, field] : classEntity->getFields()) {
        int index = constantPool.FindFieldRef(className, fieldIdentifier, field->type->toByteCode());
		context.addConstant(fieldIdentifier, new RefConstant(index, false));
		constantVisitor->getConstants(field->declaration);
    }

    for (auto & [methodIdentifier, method] : classEntity->getMethods()) {
        int index = constantPool.FindMethodRef(className, methodIdentifier, method->toTypeEntity()->toByteCode());
		context.addConstant(methodIdentifier, new RefConstant(index, false));
		constantVisitor->getConstants(method->getCodeBlock());
    }

	constantPool.FindUtf8("<clinit>");
}

void Generator::generate(std::unordered_map<std::string, ClassEntity*> & classes) {
    using namespace std::filesystem;
	create_directory(current_path() / "output");

    for (auto & [className, classEntity] : classes) {
		constantPool = ConstantPool();
		context = ContextGenerator();
		
		fillConstantPool(className, classEntity);
		addBuiltInFunctions("BuiltIn" ,Semantic::BuiltInFunctions);

        // Create class file
        const auto filename = std::string{ className } + ".class";

        auto filepath = current_path() / "output" / filename;
		
        outfile.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

		// CAFEBABE
		outfile << (char)0xCA << (char)0xFE << (char)0xBA << (char)0xBE;

		// JAVA 8 (version 52.0 (0x34))
		outfile << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x34;

		// constants count
		std::vector<char> sizeConstantPool = intToBytes(constantPool.pool.size() + 1);
		outfile << sizeConstantPool[2] << sizeConstantPool[3];

		//constants table
        for (auto & constant : constantPool.pool) {
            generateConstant(constant);
        }

		// class flags ()
		outfile << (char)0x00 << (char)0x21;

		// This class constant
		std::vector<char> bytes = intToBytes(constantPool.FindClass(className));
		outfile << bytes[2] << bytes[3];

		// Parent class constant
		bytes = intToBytes(constantPool.FindClass("java/lang/Object"));
		outfile << bytes[2] << bytes[3];

		// Interfaces table
		outfile << (char)0x00 << (char)0x00;

		// fields count
		bytes = intToBytes(classEntity->getFields().size());
		outfile << bytes[2] << bytes[3];
		
		// fields info
		for (auto & [fieldIdentifier, fieldEntity] : classEntity->getFields()) {
			generateField(fieldIdentifier, fieldEntity);
		}

		// methods count
		bytes = intToBytes(classEntity->getMethods().size() + 1);
		outfile << bytes[2] << bytes[3];

		// Generate constructor
		generateMethod("<init>", "()V", 0, uint16_t(AccessFlags::Public),
						generateGlobalClassConstructorCode());

		for (auto & [methodIdentifier, methodEntity] : classEntity->getMethods()) {
			generateMethod(methodIdentifier, methodEntity->toTypeEntity()->toByteCode()
						 , methodEntity->getNumberLocalVariables(), uint16_t(AccessFlags::Public)
																  | uint16_t(AccessFlags::Static)
						 , generateMethodBodyCode(methodEntity));
		}

        // atributes
		outfile << (char)0x00 << (char)0x00;

		outfile.close();
    }
}

void Generator::generateField(std::string fieldName, FieldEntity* field) {
	std::vector<char> bytes;
	
	// flags Public and Static
	outfile << (char)0x00 << (char)0x09;

	// field name index
	bytes = intToBytes(constantPool.FindUtf8(fieldName));
	outfile << bytes[2] << bytes[3];
	
	// field descriptor index
	bytes = intToBytes(constantPool.FindUtf8(field->type->toByteCode()));
	outfile << bytes[2] << bytes[3];

	// field attributes	(may add final flag and ConstValue attribute)
	outfile << (char)0x00 << (char)0x00;
}

void Generator::generateMethod(std::string_view methodName, std::string_view descriptor
					   , uint numeberLocals, uint accessFlags
					   , std::vector<char>&& bodyCodeBytes) {

	std::vector<char> bytes;

	// flags Public and Static
	bytes = intToBytes(accessFlags);
	outfile << bytes[2] << bytes[3];

	// field name index
	if (!methodName.empty()) {
		bytes = intToBytes(constantPool.FindUtf8(methodName));
		outfile << bytes[2] << bytes[3];
	}
	
	// field descriptor index
	bytes = intToBytes(constantPool.FindUtf8(descriptor));
	outfile << bytes[2] << bytes[3];
	
	// Only code attribute
	outfile << (char)0x00 << (char)0x01;

	// method atribute (Code - 0x01)
	bytes = intToBytes(constantPool.FindUtf8("Code"));
	outfile << bytes[2] << bytes[3];

	std::vector<char> codeAttributeBytes;

	// size of operands stack
	bytes = intToBytes(1000);
	codeAttributeBytes.push_back(bytes[2]);
	codeAttributeBytes.push_back(bytes[3]);

	// size of local variables
	bytes = intToBytes(numeberLocals);
	codeAttributeBytes.push_back(bytes[2]);
	codeAttributeBytes.push_back(bytes[3]);

	// size of code
	bytes = intToBytes(bodyCodeBytes.size());
	for (auto i : bytes) {
		codeAttributeBytes.push_back(i);
	}

	// code from bodyCodeBytes
	for (auto i : bodyCodeBytes) {
		codeAttributeBytes.push_back(i);
	}

	// exception table 
	codeAttributeBytes.push_back((char)0x00);
	codeAttributeBytes.push_back((char)0x00);

	// attrs table
	codeAttributeBytes.push_back((char)0x00);
	codeAttributeBytes.push_back((char)0x00);

	// attribute length
	bytes = intToBytes(codeAttributeBytes.size());
	for (auto i : bytes) {
		outfile << i;
	}

	// tail of attribute
	for (auto i : codeAttributeBytes) {
		outfile << i;
	}
}

std::vector<char> Generator::generateGlobalClassConstructorCode() {
	std::vector<char> bytes;

	bytes.push_back(char(Command::aload));
	bytes.push_back(char(0));

	bytes.push_back(char(Command::invokespecial));
	auto methodRefId = intToBytes(constantPool.FindMethodRef("java/lang/Object", "<init>", "()V"));
	bytes.push_back(methodRefId[2]);
	bytes.push_back(methodRefId[3]);

	bytes.push_back(char(Command::return_));
	return bytes;
}

std::vector<char> Generator::generateStaticConstuctorCode(ClassEntity* classEntity) {
	std::vector<char> bytes;
	bytes.push_back(char(Command::return_));
	return bytes;
}

std::vector<char> Generator::generateMethodBodyCode(MethodEntity* methodEntity) {
	context.addScope();
	indexCurrentLocalVariable = 0;
	currentMethod = methodEntity;

	for (auto &[name, _] : methodEntity->getArguments()) {
		context.addConstant(name, new RefConstant(indexCurrentLocalVariable++, true));
	}

	auto bytes = generate(methodEntity->getCodeBlock());

	context.popScope();
	return bytes;
}


std::vector<char> Generator::generate(BlockStatement* block) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	context.addScope();

	for (auto stmt : block->body) {
		buffer = generate(stmt);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	}

	context.popScope();
}

std::vector<char> Generator::generate(ExpressionStatement* stmt) {
	return generate(stmt->expression);
}


std::vector<char> Generator::generate(CallableExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	for (auto arg : expr->arguments) {
		buffer = generate(arg);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	}

	if (auto idExpression = dynamic_cast<IdentifierAsExpression*>(expr->base)) {
		codeBytes.push_back((char)Command::invokestatic);
		int indexFunction = context.findConstant(idExpression->identifier)->index;
		buffer = intToBytes(indexFunction);
		codeBytes.push_back(buffer[2]);
		codeBytes.push_back(buffer[3]);
	}

	return codeBytes;
}

std::vector<char> Generator::generate(IdentifierAsExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	int indexFunction = context.findConstant(expr->identifier)->index;
	buffer = intToBytes(indexFunction);
	codeBytes.push_back(buffer[2]);
	codeBytes.push_back(buffer[3]);

	return codeBytes;
}

std::vector<char> Generator::generate(StringExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes.push_back((char)Command::ldc);
	buffer = intToBytes(constantPool.FindString(expr->stringLit));
	codeBytes.push_back(buffer[2]);
	codeBytes.push_back(buffer[3]);

	return codeBytes;
}

std::vector<char> Generator::generate(ReturnStatement* expr) {
	std::vector<char> bytes;

	switch (currentMethod->getReturnType()->type) {
		case TypeEntity::TypeEntityEnum::Void: {
			bytes.push_back(char(Command::return_));
			break;
		}

		case TypeEntity::TypeEntityEnum::Int: {
			bytes.push_back(char(Command::ireturn));
			break;
		}
		default:
			break;
	}

	return bytes;
}

