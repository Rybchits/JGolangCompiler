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


void Generator::addBuiltInFunctions(std::string_view nameBaseClass, const std::unordered_map<std::string, TypeEntity*>& functions) {
	for (auto &[id, descriptor] : functions) {
		int index = constantPool.FindMethodRef(nameBaseClass, id, descriptor->toByteCode());
		context.add(id, new RefConstant(index, false));
	}	
}


void Generator::fillConstantPool(std::string_view className, ClassEntity* classEntity) {
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
		context.add(fieldIdentifier, new RefConstant(index, false));
		if (field->declaration) {
			constantVisitor->getConstants(field->declaration);
		}
    }

    for (auto & [methodIdentifier, method] : classEntity->getMethods()) {
        int index = constantPool.FindMethodRef(className, methodIdentifier, method->toTypeEntity()->toByteCode());
		context.add(methodIdentifier, new RefConstant(index, false));
		constantVisitor->getConstants(method->getCodeBlock());
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
						 , methodEntity->getNumberLocalVariables() + 1, uint16_t(AccessFlags::Public)
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
	bytes.push_back(methodRefId[2]);
	bytes.push_back(methodRefId[3]);

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
	indexCurrentLocalVariable = 1;
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

	if (expr->intLit > -2 && expr->intLit < 6) {
		codeBytes.push_back(char(Command::iconst_0) + expr->intLit);

	} else {
		codeBytes.push_back((char)Command::ldc_w);
		buffer = intToBytes(constantPool.FindInt(expr->intLit));
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
	}
	
	return codeBytes;
}


std::vector<char> Generator::generate(FloatExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes.push_back((char)Command::ldc_w);
	buffer = intToBytes(constantPool.FindFloat(expr->floatLit));
	codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

	return codeBytes;
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

	if (expr->isLogical()) {

		constexpr auto ifeqLength = 3;
        constexpr auto gotoLength = 3;
        constexpr auto iconstLength = 1;

		codeBytes = generate(expr->lhs);

		std::vector<char> rightExprBytes = generate(expr->rhs);
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

	} else {
		codeBytes = generate(expr->lhs);

		buffer = generate(expr->rhs);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

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

std::vector<char> Generator::generate(CompositeLiteral* expr) {
	return {};
}


std::vector<char> Generator::generate(ElementCompositeLiteral* expr) {
	return {};
}

std::vector<char> Generator::generate(ReturnStatement* expr) {
	std::vector<char> bytes;

	for (auto expression : expr->returnValues) {
		auto buf = generate(expression);
		bytes.insert(bytes.end(), buf.begin(), buf.end());
	}

	if (currentMethod->getReturnType()->type == TypeEntity::TypeEntityEnum::Int) {
			bytes.push_back(char(Command::ireturn));
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
		std::cout << "не сделали access expression";

	} else if (auto elementCompositeLiteral = dynamic_cast<ElementCompositeLiteral*>(expr)) {
		std::cout << "не сделали elementComposite expression";

	} else if (auto compositeLiteral = dynamic_cast<CompositeLiteral*>(expr)) {
		std::cout << "не сделали compositeLiteral expression";
		
	}

	return {};
};

std::vector<char> Generator::generate(AssignmentStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	if (stmt->type == AssignmentEnum::SimpleAssign) {
		
		auto indexIterator = stmt->indexes.begin();
        auto rightIterator = stmt->rhs.begin();
        auto leftIterator = stmt->lhs.begin();
		
		while (indexIterator != stmt->indexes.end() && leftIterator != stmt->lhs.end() 
													&& rightIterator != stmt->rhs.end()) {
			
			if (auto accessExpression = dynamic_cast<AccessExpression*>(*leftIterator)) {
				// TODO
				// right aastore index left

				//buffer = generate((*rightIterator));
				//bytes.insert(bytes.end(), buffer.begin(), buffer.end());
				
			} else if (auto identifierAsExpression = dynamic_cast<IdentifierAsExpression*>(*leftIterator)){
				buffer = generate(*rightIterator);
				bytes.insert(bytes.end(), buffer.begin(), buffer.end());

				auto constantRef = context.find(identifierAsExpression->identifier);

				if (constantRef->isLocal) {
					buffer = storeToLocalVariable(identifierAsExpression->identifier
									   , typesExpressions[identifierAsExpression->nodeId]->type);
				} else {
					bytes.push_back(char(Command::putstatic));

					buffer = intToBytes(constantRef->index);
					bytes.push_back(buffer[2]);
					bytes.push_back(buffer[3]);
				}

				bytes.insert(bytes.end(), buffer.begin(), buffer.end());		
			}

			indexIterator++;
			rightIterator++;
			leftIterator++;
		}

	} else {
		
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
		std::cout << "не сделали assignment statement";

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

