#include "codegen.h"
#include "../utils/bytes.hpp"

std::vector<char> Generator::generateConstant(Constant & constant) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	switch (constant.type)
	{
	case Constant::TypeT::Utf8: {
		char const* c = constant.utf8.c_str();
	    bytes.push_back((char)Constant::TypeT::Utf8);

		buffer = intToBytes(strlen(c));
	    bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		bytes.insert(bytes.end(), c, c + strlen(c));
		break;
	}
	case Constant::TypeT::Integer: {
		bytes.push_back((char)Constant::TypeT::Integer);

		buffer = intToBytes(constant.integer);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		break;
	}
		
	case Constant::TypeT::Float: {
		bytes.push_back((char)Constant::TypeT::Float);

		buffer = floatToBytes(constant.floating);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		break;
	}

	case Constant::TypeT::Class: {
		bytes.push_back((char)Constant::TypeT::Class);

		buffer = intToBytes(constant.classNameId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}
	
	case Constant::TypeT::String: {
		bytes.push_back((char)Constant::TypeT::String);

		buffer = intToBytes(constant.utf8Id);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}

	case Constant::TypeT::FieldRef: {
		bytes.push_back((char)Constant::TypeT::FieldRef);

		buffer = intToBytes(constant.classId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

		buffer = intToBytes(constant.nameAndTypeId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}

	case Constant::TypeT::MethodRef: {
		bytes.push_back((char)Constant::TypeT::MethodRef);
		
		buffer = intToBytes(constant.classId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

		buffer = intToBytes(constant.nameAndTypeId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}

	case Constant::TypeT::NameAndType: {
		bytes.push_back((char)Constant::TypeT::NameAndType);

		buffer = intToBytes(constant.nameId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

		buffer = intToBytes(constant.typeId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}
	}

	return bytes;
}

std::vector<char> Generator::generateInteger(int64_t number) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (number > -2 && number < 6) {
	codeBytes.push_back(char(Command::iconst_0) + number);

	} else {
		codeBytes.push_back((char)Command::ldc_w);
		buffer = intToBytes(constantPool.FindInt(number));
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
	}

	return codeBytes;
}

std::vector<char> Generator::generateFloating(float number) {
	std::vector<char> codeBytes;

	codeBytes.push_back((char)Command::ldc_w);
	std::vector<char> buffer = intToBytes(constantPool.FindFloat(number));
	codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

	return codeBytes;
}


std::vector<char> Generator::generateNewArrayCommand(TypeEntity* elementType) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	switch (elementType->type)
	{
	case TypeEntity::Int:
		codeBytes.push_back((char)Command::newarray);
		codeBytes.push_back((char)ArrayType::Int);
		break;

	case TypeEntity::Float:
		codeBytes.push_back((char)Command::newarray);
		codeBytes.push_back((char)ArrayType::Float);
		break;

	case TypeEntity::Boolean:
		codeBytes.push_back((char)Command::newarray);
		codeBytes.push_back((char)ArrayType::Boolean);
		break;

	case TypeEntity::Array:
		codeBytes.push_back((char)Command::anewarray);
		buffer = intToBytes(constantPool.FindClass(elementType->toByteCode()));
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
		break;

	case TypeEntity::String:
		codeBytes.push_back((char)Command::anewarray);
		buffer = intToBytes(constantPool.FindClass("java/lang/String"));
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
		break;
		
	default:
		break;
	}

	return codeBytes;
}


void Generator::addBuiltInFunctions(std::string_view nameBaseClass, const std::unordered_map<std::string, TypeEntity*>& functions) {
	for (auto &[id, descriptor] : functions) {
		int index = constantPool.FindMethodRef(nameBaseClass, id, descriptor->toByteCode());
		context.add(id, new RefConstant(index, false));
	}	
}


void Generator::fillConstantPool(std::string_view className, ClassEntity* classEntity) {
		
    // Add name of "Code" attribute
    constantPool.pool.push_back(Constant::CreateUtf8("Code"));
    
    // Current class
    constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8(className)));
	constantPool.pool.push_back(Constant::CreateClass(constantPool.FindUtf8("java/lang/Object")));

    constantPool.FindMethodRef("java/lang/Object", "<init>", "()V");
     
    // Add name of name, name of types and N&T of fields
    for (auto & [fieldIdentifier, field] : classEntity->getFields()) {
        int index = constantPool.FindFieldRef(className, fieldIdentifier, field->type->toByteCode());
		context.add(fieldIdentifier, new RefConstant(index, false));
    }

    for (auto & [methodIdentifier, method] : classEntity->getMethods()) {
        int index = constantPool.FindMethodRef(className, methodIdentifier, method->toTypeEntity()->toByteCode());
		context.add(methodIdentifier, new RefConstant(index, false));
    }

	constantPool.FindUtf8("<clinit>");
}

void Generator::generate() {
    using namespace std::filesystem;
	create_directory(current_path() / "output");
	std::vector<char> buffer;
	std::vector<char> classFileTail;

    for (auto & [className, classEntity] : classes) {
		constantPool = ConstantPool();
		context = Context<RefConstant*>();
		
		fillConstantPool(className, classEntity);
		addBuiltInFunctions("$Base", Semantic::BuiltInFunctions);

        // Create class file
        const auto filename = std::string{ className } + ".class";

        auto filepath = current_path() / "output" / filename;
		
		std::fstream outfile;
        outfile.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

		// write to file class file head

		// CAFEBABE
		outfile << (char)0xCA << (char)0xFE << (char)0xBA << (char)0xBE;

		// JAVA 8 (version 52.0 (0x34))
		outfile << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x3E;

		// fill class file tail to buffer

		// class flags ()
		classFileTail = {(char)0x00, (char)0x21};

		// This class constant
		buffer = intToBytes(constantPool.FindClass(className));
		classFileTail.insert(classFileTail.end(), buffer.begin() + 2, buffer.end());

		// Parent class constant
		buffer = intToBytes(constantPool.FindClass("java/lang/Object"));
		classFileTail.insert(classFileTail.end(), buffer.begin() + 2, buffer.end());

		// Interfaces table
		classFileTail.push_back((char)0x00); classFileTail.push_back((char)0x00);

		// fields count
		buffer = intToBytes(classEntity->getFields().size());
		classFileTail.insert(classFileTail.end(), buffer.begin() + 2, buffer.end());
		
		// fields info
		for (auto & [fieldIdentifier, fieldEntity] : classEntity->getFields()) {
			buffer = generateField(fieldIdentifier, fieldEntity);
			classFileTail.insert(classFileTail.end(), buffer.begin(), buffer.end());
		}

		bool hasClassInitializationConstructor = classEntity->hasFieldsDeclaration();

		// methods count
		buffer = intToBytes(classEntity->getMethods().size() + 1 + hasClassInitializationConstructor);
		classFileTail.insert(classFileTail.end(), buffer.begin() + 2, buffer.end());

		// Generate constructor
		buffer = generateMethod("<init>", "()V", 1, uint16_t(AccessFlags::Public),
						generateGlobalClassConstructorCode());
		classFileTail.insert(classFileTail.end(), buffer.begin(), buffer.end());

		if (hasClassInitializationConstructor) {
			buffer = generateMethod("<clinit>", "()V", 0, uint16_t(AccessFlags::Static)
						  , generateStaticConstuctorCode(className, classEntity));
			classFileTail.insert(classFileTail.end(), buffer.begin(), buffer.end());
		}

		for (auto & [methodIdentifier, methodEntity] : classEntity->getMethods()) {
			buffer = generateMethod(methodIdentifier, methodEntity->toTypeEntity()->toByteCode()
						 , methodEntity->getNumberLocalVariables(), uint16_t(AccessFlags::Public)
																  | uint16_t(AccessFlags::Static)
						 , generateMethodBodyCode(methodEntity));
			classFileTail.insert(classFileTail.end(), buffer.begin(), buffer.end());
		}

		// atributes
		classFileTail.push_back((char)0x00); classFileTail.push_back((char)0x00);

		// continue to write class file head

		// constants count
		std::vector<char> sizeConstantPool = intToBytes(constantPool.pool.size() + 1);
		outfile << sizeConstantPool[2] << sizeConstantPool[3];

		//constants table
        for (auto & constant : constantPool.pool) {
            buffer = generateConstant(constant);
			outfile.write(buffer.data(), buffer.size());
        }

		// write to file filled class file tail
		outfile.write(classFileTail.data(), classFileTail.size());

		outfile.close();
    }
}

std::vector<char> Generator::generateField(std::string fieldName, FieldEntity* field) {
	std::vector<char> bytes;
	std::vector<char> buffer;
	
	// flags Public and Static
	bytes = {(char)0x00, (char)0x09};

	// field name index
	buffer = intToBytes(constantPool.FindUtf8(fieldName));
	bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
	
	// field descriptor index
	buffer = intToBytes(constantPool.FindUtf8(field->type->toByteCode()));
	bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

	// field attributes	(may add final flag and ConstValue attribute)
	bytes.push_back((char)0x00); bytes.push_back((char)0x00);
	
	return bytes;
}

std::vector<char> Generator::generateMethod(std::string_view methodName, std::string_view descriptor
					   , uint numeberLocals, uint accessFlags
					   , std::vector<char>&& bodyCodeBytes) {

	std::vector<char> bytes;
	std::vector<char> buffer;

	// flags Public and Static
	buffer = intToBytes(accessFlags);
	bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

	// field name index
	if (!methodName.empty()) {
		buffer = intToBytes(constantPool.FindUtf8(methodName));
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
	}
	
	// field descriptor index
	buffer = intToBytes(constantPool.FindUtf8(descriptor));
	bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
	
	// Only code attribute
	bytes.push_back((char)0x00); bytes.push_back((char)0x01);

	// method atribute (Code - 0x01)
	buffer = intToBytes(constantPool.FindUtf8("Code"));
	bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

	std::vector<char> codeAttributeBytes;

	// size of operands stack
	buffer = intToBytes(1000);
	codeAttributeBytes.insert(codeAttributeBytes.end(), buffer.begin() + 2, buffer.end());

	// size of local variables
	buffer = intToBytes(numeberLocals);
	codeAttributeBytes.insert(codeAttributeBytes.end(), buffer.begin() + 2, buffer.end());

	// size of code
	buffer = intToBytes(bodyCodeBytes.size());
	codeAttributeBytes.insert(codeAttributeBytes.end(), buffer.begin(), buffer.end());

	// code from bodyCodeBytes
	codeAttributeBytes.insert(codeAttributeBytes.end(), bodyCodeBytes.begin(), bodyCodeBytes.end());

	// exception table 
	codeAttributeBytes.push_back((char)0x00); codeAttributeBytes.push_back((char)0x00);

	// attrs table
	codeAttributeBytes.push_back((char)0x00); codeAttributeBytes.push_back((char)0x00);

	// attribute length
	buffer = intToBytes(codeAttributeBytes.size());
	bytes.insert(bytes.end(), buffer.begin(), buffer.end());

	// tail of attribute
	bytes.insert(bytes.end(), codeAttributeBytes.begin(), codeAttributeBytes.end());

	return bytes;
}

std::vector<char> Generator::generateGlobalClassConstructorCode() {
	std::vector<char> bytes;

	bytes.push_back(char(Command::aload));
	bytes.push_back(char(0));

	bytes.push_back(char(Command::invokespecial));
	auto methodRefId = intToBytes(constantPool.FindMethodRef("java/lang/Object", "<init>", "()V"));
	bytes.insert(bytes.end(), methodRefId.begin() + 2, methodRefId.end());

	bytes.push_back(char(Command::return_));
	return bytes;
}

std::vector<char> Generator::generateStaticConstuctorCode(std::string_view className, ClassEntity* classEntity) {
	std::vector<char> bytes;

	for (auto & [fieldIdentifier, field] : classEntity->getFields()) {
		if (!field->hasDeclaration()) continue;

		auto buffer = generate(field->declaration);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		bytes.push_back(char(Command::putstatic));

		buffer = intToBytes(constantPool.FindFieldRef(className, fieldIdentifier, field->type->toByteCode()));
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
	}

	bytes.push_back(char(Command::return_));
	return bytes;
}

std::vector<char> Generator::generateMethodBodyCode(MethodEntity* methodEntity) {
	context.pushScope();
	indexCurrentLocalVariable = 0;
	currentMethod = methodEntity;

	for (auto &[name, _] : methodEntity->getArguments()) {
		context.add(name, new RefConstant(indexCurrentLocalVariable++, true));
	}

	auto bytes = generate(methodEntity->getCodeBlock());

	if (methodEntity->getReturnType()->type == TypeEntity::TypeEntityEnum::Void) {
		bytes.push_back(char(Command::return_));
	}

	context.popScope();
	return bytes;
}


std::vector<char> Generator::generate(BlockStatement* block) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	context.pushScope();

	for (auto stmt : block->body) {
		buffer = generate(stmt);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	}

	context.popScope();
	return codeBytes;
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
		int indexFunction = context.find(idExpression->identifier)->index;
		buffer = intToBytes(indexFunction);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
	}

	return codeBytes;
}

std::vector<char> Generator::generate(IdentifierAsExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	auto refConst = context.find(expr->identifier);
	buffer = intToBytes(refConst->index);

	if (refConst->isLocal) {
		switch (typesExpressions[expr->nodeId]->type)
		{
		case TypeEntity::Boolean:
		case TypeEntity::Int:
			codeBytes.push_back((char)Command::iload);
			codeBytes.push_back(buffer[3]);
			break;

		case TypeEntity::Float:
			codeBytes.push_back((char)Command::fload);
			codeBytes.push_back(buffer[3]);
			break;

		case TypeEntity::Array:
		case TypeEntity::String:
			codeBytes.push_back((char)Command::aload);
			codeBytes.push_back(buffer[3]);
		
		default:
			break;
		}

	} else {
		codeBytes.push_back((char)Command::getstatic);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
	}

	return codeBytes;
}

std::vector<char> Generator::generate(StringExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes.push_back((char)Command::ldc_w);
	buffer = intToBytes(constantPool.FindString(expr->stringLit));
	codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

	return codeBytes;
}

std::vector<char> Generator::generate(IntegerExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (typesExpressions[expr->nodeId]->isInteger())
		buffer = generateInteger(expr->intLit);

	else if (typesExpressions[expr->nodeId]->isFloat())
		buffer = generateFloating(expr->intLit);
	
	codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	
	return codeBytes;
}


std::vector<char> Generator::generate(FloatExpression* expr) {
	return generateFloating(expr->floatLit);
}

std::vector<char> Generator::generate(BooleanExpression* expr) {
	std::vector<char> codeBytes;
	if (expr->boolLit)
		codeBytes.push_back(char(Command::iconst_1));

	else
		codeBytes.push_back(char(Command::iconst_0));

	return codeBytes;
}


std::vector<char> Generator::generate(UnaryExpression* expr) {	
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	switch (expr->type)
	{
	case UnaryMinus:

		codeBytes = generate(expr->expression);

		if (typesExpressions[expr->nodeId]->isInteger()) {
			codeBytes.push_back(char(Command::ineg));

		} else if (typesExpressions[expr->nodeId]->isFloat()) {
			codeBytes.push_back(char(Command::fneg));
		}

		break;

	case Decrement:
	case Increment:
		if (auto idExpression = dynamic_cast<IdentifierAsExpression*>(expr->expression)) {
			RefConstant* ref = context.find(idExpression->identifier);

			if (typesExpressions[idExpression->nodeId]->isInteger()) {

				if (ref->isLocal) {
					codeBytes.push_back(char(Command::iinc));
					buffer = intToBytes(ref->index);
					codeBytes.push_back(buffer[3]);
					codeBytes.push_back(uint8_t(expr->type == Increment? 1 : -1));

				} else {

					buffer = intToBytes(ref->index);

					codeBytes.push_back((char)Command::getstatic);
					codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

					codeBytes.push_back(uint8_t(Command::iconst_1));
					codeBytes.push_back(uint8_t(expr->type == Increment? Command::iadd : Command::isub));

					codeBytes.push_back((char)Command::putstatic);
					codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
				}

			} else if (typesExpressions[idExpression->nodeId]->isFloat()) {

				buffer = intToBytes(ref->index);
				codeBytes.push_back(ref->isLocal? (char)Command::fload : (char)Command::getstatic);
				
				if (!ref->isLocal) codeBytes.push_back(buffer[2]);
				codeBytes.push_back(buffer[3]);

				codeBytes.push_back(uint8_t(Command::fconst_1));
				codeBytes.push_back(uint8_t(expr->type == Increment? Command::fadd : Command::fsub));

				codeBytes.push_back(ref->isLocal? (char)Command::fstore : (char)Command::putstatic);
				if (!ref->isLocal) codeBytes.push_back(buffer[2]);
				codeBytes.push_back(buffer[3]);
			}

		} else if (auto accessExpression = dynamic_cast<AccessExpression*>(expr->expression)) {
			// TODO arrays
		}

		break;

	case UnaryNot:
		codeBytes = generate(expr->expression);
		codeBytes.push_back(uint8_t(Command::ifne));
		// TODO ifne offset
		break;

	case UnaryPlus:
		codeBytes = generate(expr->expression);
		break;
	
	default:
		break;
	}
	
	return codeBytes;
}

std::vector<char> Generator::generate(BinaryExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;
	std::vector<char> rightExprBytes;

	constexpr auto ifeqLength = 3;
	constexpr auto gotoLength = 3;
	constexpr auto iconstLength = 1;

	codeBytes 	   = generate(expr->lhs);
	rightExprBytes = generate(expr->rhs);

	if (expr->isLogical()) {

		const auto trueValueOffset = rightExprBytes.size() + ifeqLength * 2;
        const auto falseValueOffset = rightExprBytes.size() + ifeqLength * 2 + iconstLength + gotoLength;

		if (expr->type == And) {
			codeBytes.push_back(char(Command::ifeq));
			buffer = intToBytes(falseValueOffset);

		} else if (expr->type == Or) {
			codeBytes.push_back(char(Command::ifne));
			buffer = intToBytes(trueValueOffset);
		}

		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
		codeBytes.insert(codeBytes.end(), rightExprBytes.begin(), rightExprBytes.end());

		codeBytes.push_back((char)Command::ifeq);

		buffer = intToBytes(ifeqLength + gotoLength + iconstLength);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		codeBytes.push_back((char)Command::iconst_1);
		codeBytes.push_back((char)Command::goto_);

		buffer = intToBytes(gotoLength + iconstLength);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		codeBytes.push_back((char)Command::iconst_0);

	} else if (expr->isComparison()) {
		codeBytes.insert(codeBytes.end(), rightExprBytes.begin(), rightExprBytes.end());

		if (typesExpressions[expr->lhs->nodeId]->isFloat()
		 && (expr->type == Less || expr->type == LessOrEqual)) {
			codeBytes.push_back((char)Command::fcmpg);

		} else if (typesExpressions[expr->lhs->nodeId]->isFloat()) { 
			codeBytes.push_back((char)Command::fcmpl); 
		}

		switch(expr->type) {
			case Equal:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
				 	codeBytes.push_back((char)Command::if_icmpne);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::ifne);

				else
					codeBytes.push_back((char)Command::if_acmpne);
				break;

			case NotEqual:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
				 	codeBytes.push_back((char)Command::if_icmpeq);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::ifeq);

				else
					codeBytes.push_back((char)Command::if_acmpeq);
				break;

			case Greater:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
				 	codeBytes.push_back((char)Command::if_icmple);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 

					codeBytes.push_back((char)Command::ifle);
				else {
					codeBytes.push_back((char)Command::invokestatic);
					buffer = intToBytes(constantPool.FindMethodRef("$Base", "compare", "(Ljava/lang/String;Ljava/lang/String;)I"));
					codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

					codeBytes.push_back((char)Command::iconst_1);
					codeBytes.push_back((char)Command::if_icmpne);
				}
				break;

			case GreatOrEqual:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
					codeBytes.push_back((char)Command::if_icmplt);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::iflt);

				else {
					codeBytes.push_back((char)Command::invokestatic);
					buffer = intToBytes(constantPool.FindMethodRef("$Base", "compare", "(Ljava/lang/String;Ljava/lang/String;)I"));
					codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

					codeBytes.push_back((char)Command::iconst_m1);
					codeBytes.push_back((char)Command::if_icmpeq);
				}
				break;

			case Less:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
					codeBytes.push_back((char)Command::if_icmpge);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::ifge);

				else {
					codeBytes.push_back((char)Command::invokestatic);
					buffer = intToBytes(constantPool.FindMethodRef("$Base", "compare", "(Ljava/lang/String;Ljava/lang/String;)I"));
					codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

					codeBytes.push_back((char)Command::iconst_m1);
					codeBytes.push_back((char)Command::if_icmpne);
				}
				break;
				
			case LessOrEqual:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
				 	codeBytes.push_back((char)Command::if_icmpgt);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::ifgt);

				else {
					codeBytes.push_back((char)Command::invokestatic);
					buffer = intToBytes(constantPool.FindMethodRef("$Base", "compare", "(Ljava/lang/String;Ljava/lang/String;)I"));
					codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

					codeBytes.push_back((char)Command::iconst_1);
					codeBytes.push_back((char)Command::if_icmpeq);
				}
				break;

			default:
				break;
		}

		buffer = intToBytes(ifeqLength + gotoLength + iconstLength);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		codeBytes.push_back((char)Command::iconst_1);
		codeBytes.push_back((char)Command::goto_);

		buffer = intToBytes(gotoLength + iconstLength);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		codeBytes.push_back((char)Command::iconst_0);

	} else {
		codeBytes.insert(codeBytes.end(), rightExprBytes.begin(), rightExprBytes.end());

		switch (expr->type) {
		case Addition:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::iadd));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fadd));

			} else if (typesExpressions[expr->nodeId]->type == TypeEntity::String) {
				codeBytes.push_back(char(Command::invokevirtual));

				auto concatRef = constantPool.FindMethodRef("java/lang/String", "concat", "(Ljava/lang/String;)Ljava/lang/String;");
				buffer = intToBytes(concatRef);

				codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
			}
			break;

		case Subtraction:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::isub));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fsub));
			}
			break;

		case Multiplication:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::imul));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fmul));
			}
			break;

		case Division:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::idiv));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fdiv));
			}
			break;

		case Mod:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::irem));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::frem));
			}
			break;

		default:
			break;
		}
	}

	return codeBytes;
}

std::vector<char> Generator::generate(AccessExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	buffer = generate(expr->base);
	codeBytes.insert(codeBytes.begin(), buffer.begin(), buffer.end());
				
	buffer = generate(expr->accessor);
	codeBytes.insert(codeBytes.begin(), buffer.begin(), buffer.end());

	/*
		private static int[] arr2 = {1, 2, 3};
		arr2[0] = 2;

		getstatic     #7
		iconst_0
		__________________________
		private static int[][] arr2 = {{1, 2, 3}};
		arr2[0][1] = 2;

		getstatic     #7                  // Field arr2:[[I
        iconst_0
        aaload
		iconst_1
		___________________________________________
		getstatic     #7                  // Field arr2:[[[I
        iconst_0
        aaload
        iconst_0
        aaload
		iconst_1
	*/

	return codeBytes;
}


std::vector<char> Generator::generate(CompositeLiteral* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (typesExpressions[expr->nodeId]->type == TypeEntity::Array) {

		auto arraySignature = std::get<ArraySignatureEntity*>(typesExpressions[expr->nodeId]->value);
		codeBytes = generateInteger(arraySignature->dims == -1? expr->elements.size() : arraySignature->dims);

		buffer = generateNewArrayCommand(arraySignature->elementType);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		int index = 0;
		for (auto element : expr->elements) {
			codeBytes.push_back((char)Command::dup);
			
			buffer = generateInteger(index++);
			codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

			buffer = generate(element);
			codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
		}
	}

	return codeBytes;
}


std::vector<char> Generator::generate(ElementCompositeLiteral* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (std::holds_alternative<ExpressionAST *>(expr->value)) {

		auto value = std::get<ExpressionAST *>(expr->value);
        codeBytes = generate(value);

		if (typesExpressions[value->nodeId]->isInteger()) {
			codeBytes.push_back((char)Command::iastore);

		} else if (typesExpressions[value->nodeId]->isFloat()) {
			codeBytes.push_back((char)Command::fastore);

		} else if (typesExpressions[value->nodeId]->type == TypeEntity::Boolean) {
			codeBytes.push_back((char)Command::bastore);

		} else {
			codeBytes.push_back((char)Command::aastore);
		}

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral*>>(expr->value)) {

        auto elements = std::get<std::list<ElementCompositeLiteral*>>(expr->value);
		auto arrayType = std::get<ArraySignatureEntity*>(typesExpressions[expr->nodeId]->value);

		codeBytes = generateInteger(arrayType->dims);

		buffer = generateNewArrayCommand(arrayType->elementType);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		int index = 0;
		for (auto element : elements) {

			codeBytes.push_back((char)Command::dup);

			buffer = generateInteger(index++);
			codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

			buffer = generate(element);
			codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
		}
		codeBytes.push_back((char)Command::aastore);
    }

	return codeBytes;
}

std::vector<char> Generator::generate(ReturnStatement* expr) {
	std::vector<char> bytes;

	for (auto expression : expr->returnValues) {
		auto buf = generate(expression);
		bytes.insert(bytes.end(), buf.begin(), buf.end());
	}

	if (currentMethod->getReturnType()->isInteger() || currentMethod->getReturnType()->type == TypeEntity::Boolean) {
		bytes.push_back(char(Command::ireturn));

	} else if (currentMethod->getReturnType()->isFloat()) {
		bytes.push_back(char(Command::freturn));

	}  else {
		bytes.push_back(char(Command::areturn));
	}

	return bytes;
}


std::vector<char> Generator::initializeLocalVariables(const IdentifiersList& identifiers, const ExpressionList& values) {
	std::vector<char> bytes;
	std::vector<char> buffer;
	
	auto idIter = identifiers.begin();
	auto valueIter = values.begin();

	while (idIter != identifiers.end() && valueIter != values.end()) {
			
		buffer = generate((*valueIter));
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		
		buffer = storeToLocalVariable(*idIter, typesExpressions[(*valueIter)->nodeId]->type);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());

		idIter++;
		valueIter++;
	}

	return bytes;
}

std::vector<char> Generator::generate(DeclarationStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	for (auto decl : stmt->declarations) {
		if (auto varDecl = dynamic_cast<VariableDeclaration*>(decl)) {
			for (auto id : varDecl->identifiersWithType->identifiers) {
				context.add(id, new RefConstant(indexCurrentLocalVariable++, true));
			}

			buffer = initializeLocalVariables(varDecl->identifiersWithType->identifiers, varDecl->values);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		}
	}
	
	return bytes;
}

std::vector<char> Generator::generate(ShortVarDeclarationStatement* stmt) {
	for (auto id : stmt->identifiers) {
		context.add(id, new RefConstant(indexCurrentLocalVariable++, true));
	}

	return initializeLocalVariables(stmt->identifiers, stmt->values);;
}


std::vector<char> Generator::generate(AssignmentStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	if (stmt->type == AssignmentEnum::SimpleAssign) {
		
		auto indexIterator = stmt->indexes.begin();
        auto rightIterator = stmt->rhs.begin();
        auto leftIterator = stmt->lhs.begin();
		
		while (indexIterator != stmt->indexes.end() && leftIterator != stmt->lhs.end() 
													&& rightIterator != stmt->rhs.end()) {

			/*
				AccessExpression:
				0: getstatic     #7                  // Field arr2:[I	// это самый нижний id base
         		3: iconst_1												// index
         		4: sipush        222									// value
         		7: iastore												// store в соотвествии с типом

				global IdentifierAsType:
				0: sipush        222									// value
         		3: putstatic     #7                  // Field arr2:I	// store в соотвествии с типом

         		2: sipush        222									// value
         		5: istore #1											// store в соотвествии с типом
			*/

			auto accessExpression = dynamic_cast<AccessExpression*>(*leftIterator);
			auto identifierAsExpression = dynamic_cast<IdentifierAsExpression*>(*leftIterator);

			if (accessExpression) {
				buffer = generate(*leftIterator);
				bytes.insert(bytes.end(), buffer.begin(), buffer.end());
			}

			buffer = generate(*rightIterator);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());

			// Если accessExpression или локальный identifierAsExpression

			// Здесь надо понять, что происходит с storeToLocalVariable
			// т.к. там предполагается загрузка в identifierAsExpression
			// а нужно что-то сделать для AccessExpression (aastore, iastore и т.п.)
			if (accessExpression 
			|| (identifierAsExpression && context.find(identifierAsExpression->identifier)->isLocal)) {

				auto identifier = identifierAsExpression? identifierAsExpression->identifier : "";

				buffer = storeToLocalVariable(identifier
									, typesExpressions[(*leftIterator)->nodeId]->type);

				bytes.insert(bytes.end(), buffer.begin(), buffer.end());

			} else {	// Только для identifierAsExpression
				bytes.push_back(char(Command::putstatic));

				buffer = intToBytes(context.find(identifierAsExpression->identifier)->index);
				bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
			}

			indexIterator++;
			rightIterator++;
			leftIterator++;
		}

	} else {
		// TODO different assignments
	}

	return bytes;
}

std::vector<char> Generator::storeToLocalVariable(std::string variableIdentifier, TypeEntity::TypeEntityEnum type) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	switch (type)
	{
		case TypeEntity::Boolean:
		case TypeEntity::UntypedInt:
		case TypeEntity::Int:
			bytes.push_back((char)Command::istore);
			buffer = intToBytes(context.find(variableIdentifier)->index);
			bytes.push_back(buffer[3]);
			break;
		case TypeEntity::Array:
		case TypeEntity::String:
			bytes.push_back((char)Command::astore);
			buffer = intToBytes(context.find(variableIdentifier)->index);
			bytes.push_back(buffer[3]);
			break;

		case TypeEntity::UntypedFloat:
		case TypeEntity::Float:
			bytes.push_back((char)Command::fstore);
			buffer = intToBytes(context.find(variableIdentifier)->index);
			bytes.push_back(buffer[3]);
			break;

		default:
			break;
	}

	return bytes;
}

std::vector<char> Generator::generate(StatementAST* stmt) {

	if (auto expressionStatement = dynamic_cast<ExpressionStatement*>(stmt)) {
		return generate(expressionStatement);

	} else if (auto returnStatement = dynamic_cast<ReturnStatement*>(stmt)) {
		return generate(returnStatement);
		
	} else if (auto blockStatement = dynamic_cast<BlockStatement*>(stmt)) {
		return generate(blockStatement);

	} else if (auto declarationStatement = dynamic_cast<DeclarationStatement*>(stmt)) {
		return generate(declarationStatement);

	} else if (auto shortDeclStatement = dynamic_cast<ShortVarDeclarationStatement*>(stmt)) {
		return generate(shortDeclStatement);

	} else if (auto keywordStatement = dynamic_cast<KeywordStatement*>(stmt)) {
		// return generate(keywordStatement);
		std::cout << "не сделали keyword statement";

	} else if (auto assignmentStatement = dynamic_cast<AssignmentStatement*>(stmt)) {
		return generate(assignmentStatement);

	} else if (auto whileStatement = dynamic_cast<WhileStatement*>(stmt)) {
		// return generate(assignmentStatement);
		std::cout << "не сделали while statement";

	} else if (auto ifStatement = dynamic_cast<IfStatement*>(stmt)) {
		// return generate(assignmentStatement);
		std::cout << "не сделали if statement";

	} else if (auto switchCaseClause = dynamic_cast<SwitchCaseClause*>(stmt)) {
		// return generate(assignmentStatement);
		std::cout << "не сделали switch case clause statement";

	} else if (auto switchStatement = dynamic_cast<SwitchStatement*>(stmt)) {
		// return generate(assignmentStatement);
		std::cout << "не сделали switch statement";
	}

	return {};
};

// TODO make as visitor
std::vector<char> Generator::generate(ExpressionAST* expr) { 
	if (auto stringExpr = dynamic_cast<StringExpression*>(expr)) {
		return generate(stringExpr);

	} else if (auto integerExpression = dynamic_cast<IntegerExpression*>(expr)) {
		return generate(integerExpression);

	} else if (auto floatExpression = dynamic_cast<FloatExpression*>(expr)) {
		return generate(floatExpression);

	} else if (auto callableExpression = dynamic_cast<CallableExpression*>(expr)) {
		return generate(callableExpression);
		
	} else if (auto identifierAsExpression = dynamic_cast<IdentifierAsExpression*>(expr)) {
		return generate(identifierAsExpression);

	}  else if (auto booleanExpression = dynamic_cast<BooleanExpression*>(expr)) {
		return generate(booleanExpression);

	} else if (auto runeExpression = dynamic_cast<RuneExpression*>(expr)) {
		std::cout << "Rune expression is not exists";
		
	} else if (auto unaryExpression = dynamic_cast<UnaryExpression*>(expr)) {
		return generate(unaryExpression);

	} else if (auto binaryExpression = dynamic_cast<BinaryExpression*>(expr)) {
		return generate(binaryExpression);

	} else if (auto accessExpression = dynamic_cast<AccessExpression*>(expr)) {
		return generate(accessExpression);

	} else if (auto elementCompositeLiteral = dynamic_cast<ElementCompositeLiteral*>(expr)) {
		return generate(elementCompositeLiteral);

	} else if (auto compositeLiteral = dynamic_cast<CompositeLiteral*>(expr)) {
		return generate(compositeLiteral);
	}

	return {};
};
