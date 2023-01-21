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

// If is not array returns empty vector bytes
std::vector<char> Generator::generateCloneArrayCommand(ExpressionAST* array) {
	std::vector<char> codeBytes;

	auto arrayType = typesExpressions[array->nodeId];

	if (arrayType->type == TypeEntity::Array 
			&& !std::get<ArraySignatureEntity*>(arrayType->value)->isSlice() && !dynamic_cast<CompositeLiteral*>(array)) {

		codeBytes.push_back(char(Command::invokevirtual));

		std::vector<char> buffer = intToBytes(constantPool.FindMethodRef(arrayType->toByteCode(), "clone", "()Ljava/lang/Object;"));
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
	}

	return codeBytes;
}

std::vector<char> Generator::generateNewArray(ArraySignatureEntity* arrayType, ElementCompositeLiteralList elements) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes = generateInteger(arrayType->dims == -1? elements.size() : arrayType->dims);

	buffer = generateNewArrayCommand(arrayType->elementType);
	codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

	int index = 0;
	for (auto element : elements) 
	{
		codeBytes.push_back((char)Command::dup);
			
		buffer = generateInteger(index++);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		buffer = generate(element);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	}

	bool isObjectNeedToInitialize = arrayType->elementType->type == TypeEntity::Array || arrayType->elementType->type == TypeEntity::String;
	for (; index < arrayType->dims && isObjectNeedToInitialize; index++)
	{
		codeBytes.push_back((char)Command::dup);
			
		buffer = generateInteger(index);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		if (arrayType->elementType->type == TypeEntity::Array) {
			buffer = generateNewArray(std::get<ArraySignatureEntity*>(arrayType->elementType->value), ElementCompositeLiteralList({}));
			codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		} else if (arrayType->elementType->type == TypeEntity::String) {
			codeBytes.push_back(char(Command::new_));

			buffer = intToBytes(constantPool.FindClass("java/lang/String"));
			codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

			codeBytes.push_back(char(Command::dup));
			
			codeBytes.push_back(char(Command::invokespecial));
			buffer = intToBytes(constantPool.FindMethodRef("java/lang/String", "<init>", "()V"));
			codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
		}

		codeBytes.push_back((char)Command::aastore);
	}

	return codeBytes;
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

		buffer = generateCloneArrayCommand(field->declaration);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		
		buffer = generateStoreToVariableCommand(fieldIdentifier, typesExpressions[field->declaration->nodeId]->type);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
	}

	bytes.push_back(char(Command::return_));
	return bytes;
}

std::vector<char> Generator::generateMethodBodyCode(MethodEntity* methodEntity) {
	context.pushScope();
	indexCurrentLocalVariable = 0;
	currentMethod = methodEntity;

	for (auto &[name, _] : methodEntity->getArguments()) {
		if (context.add(name, new RefConstant(indexCurrentLocalVariable, true))) {
			indexCurrentLocalVariable++;
		}
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


std::vector<char> Generator::generate(IfStatement* stmt) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	constexpr auto gotoLength = 3;
	constexpr auto ifeqLength = 3;

    auto conditionBytes = generate(stmt->condition);
    auto thenStmtBytes = generate(stmt->thenStatement);
    auto elseStmtBytes = generate(stmt->elseStatement);

	codeBytes.insert(codeBytes.end(), conditionBytes.begin(), conditionBytes.end());

	if (stmt->elseStatement != nullptr) {
		thenStmtBytes.push_back(char(Command::goto_));

		buffer = intToBytes(elseStmtBytes.size() + gotoLength);
		thenStmtBytes.insert(thenStmtBytes.end(), buffer.begin() + 2, buffer.end());
	}

	codeBytes.push_back((char)Command::ifeq);

	buffer = intToBytes(thenStmtBytes.size() + ifeqLength);
	codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

	codeBytes.insert(codeBytes.end(), thenStmtBytes.begin(), thenStmtBytes.end());
	codeBytes.insert(codeBytes.end(), elseStmtBytes.begin(), elseStmtBytes.end());

	codeBytes.push_back((char)Command::nop);

	return codeBytes;
}


std::vector<char> Generator::generate(ExpressionStatement* stmt) {
	return generate(stmt->expression);
}

std::string Generator::createDescriptorBuiltInFunction(CallableExpression* expr) {
	std::string nameFunction = dynamic_cast<IdentifierAsExpression*>(expr->base)->identifier;
	
	std::string descriptor = "(";

	int index = 0;
	for (auto arg : expr->arguments) {
		
		TypeEntity* typeArgument = typesExpressions[arg->nodeId];
		ArraySignatureEntity** arrayArgument = std::get_if<ArraySignatureEntity*>(&(typeArgument->value));

		if (arrayArgument && ((*arrayArgument)->elementType->type == TypeEntity::String 
			|| (*arrayArgument)->elementType->type == TypeEntity::Array)) {

			descriptor += "[Ljava/lang/Object;";

		} else if (nameFunction != "len" && typeArgument->type == TypeEntity::String) {
			descriptor += "Ljava/lang/Object;";

		} else if (nameFunction == "append" && index == 1 && typeArgument->type == TypeEntity::Array) {
			descriptor += "Ljava/lang/Object;";

		} else {
			descriptor += typeArgument->toByteCode();
		}

		index++;
    }

	descriptor += ")";

	auto returnType = typesExpressions[expr->nodeId];

	if (returnType->type == TypeEntity::String) {
		descriptor += "Ljava/lang/Object;";

	} else if (returnType->type == TypeEntity::Array 
				&& (std::get<ArraySignatureEntity*>(returnType->value)->elementType->type == TypeEntity::String
				|| std::get<ArraySignatureEntity*>(returnType->value)->elementType->type == TypeEntity::Array)) {

		descriptor += "[Ljava/lang/Object;";
		
	} else {
		descriptor += returnType->toByteCode();
	}

	return descriptor;
}

std::vector<char> Generator::generate(CallableExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	for (auto arg : expr->arguments) {
		buffer = generate(arg);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		buffer = generateCloneArrayCommand(arg);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	}
	

	if (auto idExpression = dynamic_cast<IdentifierAsExpression*>(expr->base)) {
		codeBytes.push_back((char)Command::invokestatic);

		RefConstant* methodRef = context.find(idExpression->identifier);

		if (methodRef != nullptr) {
			buffer = intToBytes(methodRef->index);
			codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		} else if (Semantic::IsBuiltInFunction(idExpression->identifier)) {
			std::string descriptor = createDescriptorBuiltInFunction(expr);

			int indexBuiltInFunction = constantPool.FindMethodRef("$Base", idExpression->identifier, descriptor);
			buffer = intToBytes(indexBuiltInFunction);
			codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());
		}
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
	case UnaryExpression::UnaryMinus:
		codeBytes = generate(expr->expression);

		if (typesExpressions[expr->nodeId]->isInteger()) {
			codeBytes.push_back(char(Command::ineg));

		} else if (typesExpressions[expr->nodeId]->isFloat()) {
			codeBytes.push_back(char(Command::fneg));
		}

		break;

	case UnaryExpression::Decrement:
	case UnaryExpression::Increment:
		buffer = generate(expr->expression);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		// dirty (duplicate array for loading and storing)
		if (dynamic_cast<AccessExpression*>(expr->expression)) {
			codeBytes.insert(codeBytes.end() - 1, (char)Command::dup2);
		}

		if (typesExpressions[expr->expression->nodeId]->isInteger()) {
			codeBytes.push_back(uint8_t(Command::iconst_1));
			codeBytes.push_back(uint8_t(expr->type == UnaryExpression::Increment? Command::iadd : Command::isub));
			
		} else if (typesExpressions[expr->expression->nodeId]->isFloat()) {
			codeBytes.push_back(uint8_t(Command::fconst_1));
			codeBytes.push_back(uint8_t(expr->type == UnaryExpression::Increment? Command::fadd : Command::fsub));
		}
		
		if (auto identifierAsExpression = dynamic_cast<IdentifierAsExpression*>(expr->expression)) {
			buffer = generateStoreToVariableCommand(identifierAsExpression->identifier
											, typesExpressions[identifierAsExpression->nodeId]->type);

		} else if (auto accessExpression = dynamic_cast<AccessExpression*>(expr->expression)) {
			
			buffer = generateStoreToArrayCommand(typesExpressions[accessExpression->nodeId]->type);
		}

		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
		break;

	case UnaryExpression::UnaryNot: {
		constexpr auto ifeqLength = 3;
		constexpr auto gotoLength = 3;
		constexpr auto iconstLength = 1;

		codeBytes = generate(expr->expression);
		codeBytes.push_back((char)Command::iconst_1);
		codeBytes.push_back((char)Command::if_icmpeq);

		buffer = intToBytes(ifeqLength + gotoLength + iconstLength);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		codeBytes.push_back((char)Command::iconst_1);

		codeBytes.push_back((char)Command::goto_);
		buffer = intToBytes(gotoLength + iconstLength);
		codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		codeBytes.push_back((char)Command::iconst_0);

		break;
	}

	case UnaryExpression::UnaryPlus:
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

		if (expr->type == BinaryExpression::And) {
			codeBytes.push_back(char(Command::ifeq));
			buffer = intToBytes(falseValueOffset);

		} else if (expr->type == BinaryExpression::Or) {
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
		 && (expr->type == BinaryExpression::Less || expr->type == BinaryExpression::LessOrEqual)) {
			codeBytes.push_back((char)Command::fcmpg);

		} else if (typesExpressions[expr->lhs->nodeId]->isFloat()) { 
			codeBytes.push_back((char)Command::fcmpl); 
		}
		
		if (typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Array) {
			auto arrayEntity = typesExpressions[expr->lhs->nodeId];
			auto arrayElementType = std::get<ArraySignatureEntity*>(arrayEntity->value)->elementType;

			codeBytes.push_back((char)Command::invokestatic);
			if (arrayElementType->isInteger()) {
				buffer = intToBytes(constantPool.FindMethodRef("$Base", "equals", "([I[I)Z"));

			} else if (arrayElementType->type == TypeEntity::Boolean) {
				buffer = intToBytes(constantPool.FindMethodRef("$Base", "equals", "([Z[Z)Z"));

			} else if (arrayElementType->isFloat()) {
				buffer = intToBytes(constantPool.FindMethodRef("$Base", "equals", "([F[F)Z"));
				
			} else {
				buffer = intToBytes(constantPool.FindMethodRef("$Base", "equals", "([Ljava/lang/Object;[Ljava/lang/Object;)Z"));
			}

			codeBytes.insert(codeBytes.end(), buffer.begin() + 2, buffer.end());

		}

		switch(expr->type) {

			case BinaryExpression::Equal:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
				 	codeBytes.push_back((char)Command::if_icmpne);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::ifne);

				else if (typesExpressions[expr->lhs->nodeId]->type == TypeEntity::String)
					codeBytes.push_back((char)Command::if_acmpne);

				else if (typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Array) {
					codeBytes.push_back((char)Command::iconst_1);
					codeBytes.push_back((char)Command::if_icmpne);
				}

				break;

			case BinaryExpression::NotEqual:
				if (typesExpressions[expr->lhs->nodeId]->isInteger() 
				 || typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Boolean)
				 	codeBytes.push_back((char)Command::if_icmpeq);

				else if (typesExpressions[expr->lhs->nodeId]->isFloat()) 
					codeBytes.push_back((char)Command::ifeq);

				else if (typesExpressions[expr->lhs->nodeId]->type == TypeEntity::String)
					codeBytes.push_back((char)Command::if_acmpeq);

				else if (typesExpressions[expr->lhs->nodeId]->type == TypeEntity::Array) {
					codeBytes.push_back((char)Command::iconst_1);
					codeBytes.push_back((char)Command::if_icmpeq);
				}

				break;

			case BinaryExpression::Greater:
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

			case BinaryExpression::GreatOrEqual:
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

			case BinaryExpression::Less:
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
				
			case BinaryExpression::LessOrEqual:
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
		case BinaryExpression::Addition:
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

		case BinaryExpression::Subtraction:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::isub));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fsub));
			}
			break;

		case BinaryExpression::Multiplication:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::imul));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fmul));
			}
			break;

		case BinaryExpression::Division:
			if (typesExpressions[expr->nodeId]->isInteger()) {
				codeBytes.push_back(char(Command::idiv));

			} else if (typesExpressions[expr->nodeId]->isFloat()) {
				codeBytes.push_back(char(Command::fdiv));
			}
			break;

		case BinaryExpression::Mod:
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

	if (expr->type == AccessExpression::Indexing) {

		buffer = generate(expr->base);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		buffer = generate(expr->accessor);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		buffer = generateLoadFromArrayCommand(typesExpressions[expr->nodeId]->type);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());
	}

	return codeBytes;
}


std::vector<char> Generator::generate(CompositeLiteral* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (typesExpressions[expr->nodeId]->type == TypeEntity::Array) {

		auto arraySignature = std::get<ArraySignatureEntity*>(typesExpressions[expr->nodeId]->value);
		codeBytes = generateNewArray(arraySignature, expr->elements);
	}

	return codeBytes;
}


std::vector<char> Generator::generate(ElementCompositeLiteral* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (std::holds_alternative<ExpressionAST *>(expr->value)) {

		auto value = std::get<ExpressionAST *>(expr->value);
        codeBytes = generate(value);

		buffer = generateCloneArrayCommand(value);;
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

		buffer = generateStoreToArrayCommand(typesExpressions[value->nodeId]->type);
		codeBytes.insert(codeBytes.end(), buffer.begin(), buffer.end());

    } else if (std::holds_alternative<ElementCompositeLiteralList>(expr->value)) {

        auto elements = std::get<ElementCompositeLiteralList>(expr->value);
		auto arrayType = std::get<ArraySignatureEntity*>(typesExpressions[expr->nodeId]->value);

		codeBytes = generateNewArray(arrayType, elements);
		codeBytes.push_back((char)Command::aastore);
    }

	return codeBytes;
}

std::vector<char> Generator::generate(ReturnStatement* expr) {
	std::vector<char> bytes;

	for (auto expression : expr->returnValues) {
		auto buffer = generate(expression);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());

		buffer = generateCloneArrayCommand(expression);;
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
	}

	if (currentMethod->getReturnType()->type == TypeEntity::Void) {
		bytes.push_back(char(Command::return_));

	} else if (currentMethod->getReturnType()->isInteger() || currentMethod->getReturnType()->type == TypeEntity::Boolean) {
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

		buffer = generateCloneArrayCommand(*valueIter);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		
		if ((*idIter) == "_") {
			bytes.push_back(char(Command::pop));
			
		} else {
			buffer = generateStoreToVariableCommand(*idIter, typesExpressions[(*valueIter)->nodeId]->type);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		}

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

				if (context.add(id, new RefConstant(indexCurrentLocalVariable, true)))
					indexCurrentLocalVariable++;
			}

			buffer = initializeLocalVariables(varDecl->identifiersWithType->identifiers, varDecl->values);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		}
	}
	
	return bytes;
}

std::vector<char> Generator::generate(ShortVarDeclarationStatement* stmt) {
	for (auto id : stmt->identifiers) {
		if (context.add(id, new RefConstant(indexCurrentLocalVariable, true))) {
			indexCurrentLocalVariable++;
		}
	}

	return initializeLocalVariables(stmt->identifiers, stmt->values);
}


std::vector<char> Generator::generate(AssignmentStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	if (stmt->type == AssignmentStatement::SimpleAssign) {
		
		auto indexRIterator = stmt->indexes.rbegin();
        auto rightRIterator = stmt->rhs.rbegin();
        auto leftRIterator = stmt->lhs.rbegin();

		while (indexRIterator != stmt->indexes.rend() && leftRIterator != stmt->lhs.rend() 
													&& rightRIterator != stmt->rhs.rend()) {
			// if lhs expression has index (i.e. array)
			if (*indexRIterator) {
				// generate access array expression
				buffer = generate(*leftRIterator);
				bytes.insert(bytes.end(), buffer.begin(), buffer.end());

				// generate index expression
				buffer = generate(*indexRIterator);
				bytes.insert(bytes.end(), buffer.begin(), buffer.end());
			}

			// generate value expression
			buffer = generate(*rightRIterator);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());

			buffer = generateCloneArrayCommand(*rightRIterator);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());

			indexRIterator++;
			rightRIterator++;
			leftRIterator++;
		}

		auto indexIterator = stmt->indexes.begin();
        auto rightIterator = stmt->rhs.begin();
        auto leftIterator = stmt->lhs.begin();
		
		while (indexIterator != stmt->indexes.end() && leftIterator != stmt->lhs.end() 
													&& rightIterator != stmt->rhs.end()) {

			auto identifierAsExpression = dynamic_cast<IdentifierAsExpression*>(*leftIterator);

			if (*indexIterator) {
				buffer = generateStoreToArrayCommand(typesExpressions[(*rightIterator)->nodeId]->type);
				bytes.insert(bytes.end(), buffer.begin(), buffer.end());
			}
			else if (identifierAsExpression) {
				auto identifier = identifierAsExpression->identifier;

				if (identifier == "_") {
					bytes.push_back(char(Command::pop));

				} else {
					buffer = generateStoreToVariableCommand(identifier
								, typesExpressions[(*leftIterator)->nodeId]->type);

					bytes.insert(bytes.end(), buffer.begin(), buffer.end());
				}
			}

			indexIterator++;
			rightIterator++;
			leftIterator++;
		}

	}

	return bytes;
}

std::vector<char> Generator::generate(SwitchStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	SwitchCaseClause * defaultClause = nullptr;

	// Генерируем условия для прыжков (cases conditions)
	std::vector<std::vector<char>> clausesConditionsToJump;
	std::vector<size_t> conditionsOffsets = {0};

	for (const auto clause : stmt->clauseList) {
		std::vector<char> clauseConditionToJump;

		if (clause->expressionCase == nullptr) {
			defaultClause = clause;
			continue;
		}

		// Сравниваем switch expression и case expression
		auto conditionBytes = generate(
			std::make_unique<BinaryExpression>(
				BinaryExpression(BinaryExpression::Equal, stmt->expression, clause->expressionCase)
			).get()
		);

		clauseConditionToJump.insert(clauseConditionToJump.end(), conditionBytes.begin(), conditionBytes.end());

		// Если текущий кейс подходящий
		clauseConditionToJump.push_back((char)Command::ifne);

		// Прыгаем на тело кейса (пока не знаем сдвиг, запишем филлеры)
		buffer = {(char)0xF6, (char)0xF6};
		clauseConditionToJump.insert(clauseConditionToJump.end(), buffer.begin(), buffer.end());

		clausesConditionsToJump.push_back(clauseConditionToJump);

		conditionsOffsets.push_back(conditionsOffsets.back() + clauseConditionToJump.size());

		// Иначе смотрим следующий case condition
	}

	// Добавляем прыжок на тело дефолта
	buffer = std::vector<char>{(char)Command::ifne, (char)0xF6, (char)0xF6};
	clausesConditionsToJump.push_back(buffer);
	conditionsOffsets.push_back(conditionsOffsets.back() + buffer.size());

	conditionsOffsets.erase(conditionsOffsets.begin());

	// Считаем сдвиг от прыжка из каждого условия (case condition) до конца всех условий 
	size_t conditionsLastByte = conditionsOffsets.back();
	std::for_each(conditionsOffsets.begin(), conditionsOffsets.end(), [&](auto & e){ e = conditionsLastByte - e + 3; });


	// Запишем тела кейсов
	std::vector<std::vector<char>> clausesBlocks;

	// Сдвиг для каждого тела кейса относительно начала части тел (кроме дефолта)
	std::vector<size_t> blocksOffsets = {0};

	// Тела кейсов, заданных пользователем
	for (const auto clause : stmt->clauseList) {

		buffer = generate(clause->block);

		// Если стоит фолтру, проваливаемся к телу следующего кейса
		// Если не стоит - прыгаем в конец switch statement (пока не знаем, запишем филлеры)
		if (!clause->fallthrowEnds && clause != stmt->clauseList.back()) {
			buffer.push_back((char)BREAK_FILLER);
			buffer.push_back((char)BREAK_FILLER);
			buffer.push_back((char)BREAK_FILLER);
		}

		clausesBlocks.push_back(buffer);
		blocksOffsets.push_back(blocksOffsets.back() + buffer.size());
	}

	// Если дефолт кейс стоит не в конце
	if (defaultClause != nullptr) {
		// Вычисляем позицию дефолт кейса в списке
		size_t defaultClauseI = 
			std::distance(stmt->clauseList.begin()
						, std::find(stmt->clauseList.begin(), stmt->clauseList.end(), defaultClause));

		// Перемещаем сдвиг для него в конец списка сдвигов, т.к. default condition стоит в конце
		// Ставим его вместо сдвига, который указывает на nop
		auto defaultClauseBlockOffset = blocksOffsets[defaultClauseI];
		blocksOffsets.erase(blocksOffsets.begin() + defaultClauseI);
		blocksOffsets.back() = defaultClauseBlockOffset;
	}
	// Иначе считаем, что из default condition должны прыгнуть на nop

	// Cчитаем сдвиги для прыжков из условий кейсов в тела
	for (size_t i = 0; i < clausesConditionsToJump.size(); ++i) {
		buffer = intToBytes(conditionsOffsets[i] + blocksOffsets[i]);
		if (i == clausesConditionsToJump.size() - 1) {
			clausesConditionsToJump[i][clausesConditionsToJump[i].size() - 3] = (char)Command::goto_;
		}
		clausesConditionsToJump[i][clausesConditionsToJump[i].size() - 2] = buffer[2];
		clausesConditionsToJump[i][clausesConditionsToJump[i].size() - 1] = buffer[3];
	}

	for (auto & condition : clausesConditionsToJump) {
		bytes.insert(bytes.end(), condition.begin(), condition.end());
	}

	for (auto & block : clausesBlocks) {
		bytes.insert(bytes.end(), block.begin(), block.end());
	}

	bytes.push_back((char)Command::nop);

	replaceBreakFillersWithGotoInBlockCodeBytes(bytes);

	return bytes;
}

std::vector<char> Generator::generate(WhileStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	const auto conditionBytes = generate(stmt->conditionExpression);
    const auto bodyBytes = generate(stmt->block);

    constexpr auto ifeqCommandLength = 3;
    constexpr auto gotoCommandLength = 3;

	bytes.insert(bytes.end(), conditionBytes.begin(), conditionBytes.end());

	bytes.push_back((char)Command::ifeq);
	buffer = intToBytes(bodyBytes.size() + ifeqCommandLength + gotoCommandLength);
    bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

	bytes.insert(bytes.end(), bodyBytes.begin(), bodyBytes.end());

	bytes.push_back((char)Command::goto_);
	buffer = intToBytes(-(conditionBytes.size() + bodyBytes.size() + ifeqCommandLength));
    bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

	bytes.push_back((char)Command::nop);
	
	replaceBreakFillersWithGotoInBlockCodeBytes(bytes);
	replaceContinueFillersWithGotoInBlockCodeBytes(bytes);

	return bytes; 
}

std::vector<char> Generator::generate(KeywordStatement* stmt) {
	std::vector<char> bytes;
	std::vector<char> buffer;
	
	switch(stmt->type) {
		case KeywordStatement::Break:
			bytes = std::vector<char>(3, BREAK_FILLER); 
			break;

		case KeywordStatement::Continue:
			bytes = std::vector<char>(3, CONTINUE_FILLER);
			break;
			
		default:
			break;
	}

	return bytes;
}

void Generator::replaceContinueFillersWithGotoInBlockCodeBytes(std::vector<char>& bytes) {
	std::vector<char> buffer;

	for (int i = 0; i < bytes.size() - 3; ++i) {
		if (bytes[i]   == CONTINUE_FILLER 
		 && bytes[i+1] == CONTINUE_FILLER 
		 && bytes[i+2] == CONTINUE_FILLER) {
			// this is CONTINUE filler
			// Need to fill with goto_ and shift count for jumping to start of statement
			bytes[i] = (char)Command::goto_;
			buffer = intToBytes(-i);
			bytes[i+1] = buffer[2];
			bytes[i+2] = buffer[3];
		}
	}
}

void Generator::replaceBreakFillersWithGotoInBlockCodeBytes(std::vector<char>& bytes) {
	std::vector<char> buffer;

	for (int i = 0; i < bytes.size() - 2; ++i) {
		if (bytes[i]   == BREAK_FILLER 
		 && bytes[i+1] == BREAK_FILLER 
		 && bytes[i+2] == BREAK_FILLER) {
			// this is BREAK filler
			// Need to fill with goto_ and shift count for jumping to end of statement
			bytes[i] = (char)Command::goto_;
			buffer = intToBytes(bytes.size() - i - 1);
			bytes[i+1] = buffer[2];
			bytes[i+2] = buffer[3];
		}
	}
}

std::vector<char> Generator::generateStoreToVariableCommand(std::string variableIdentifier, TypeEntity::TypeEntityEnum type) {
	std::vector<char> bytes;
	std::vector<char> buffer;

	auto constRef = context.find(variableIdentifier);

	if (constRef->isLocal == false) {
		bytes.push_back((char)Command::putstatic);
		buffer = intToBytes(constRef->index);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

	} else {
		switch (type)
		{
		case TypeEntity::Boolean:
		case TypeEntity::UntypedInt:
		case TypeEntity::Int:
			bytes.push_back((char)Command::istore);
			buffer = intToBytes(constRef->index);
			bytes.push_back(buffer[3]);
			break;
		case TypeEntity::Array:
		case TypeEntity::String:
			bytes.push_back((char)Command::astore);
			buffer = intToBytes(constRef->index);
			bytes.push_back(buffer[3]);
			break;

		case TypeEntity::UntypedFloat:
		case TypeEntity::Float:
			bytes.push_back((char)Command::fstore);
			buffer = intToBytes(constRef->index);
			bytes.push_back(buffer[3]);
			break;

		default:
			break;
		}
	}

	return bytes;
}

std::vector<char> Generator::generateStoreToArrayCommand(TypeEntity::TypeEntityEnum type) {
	std::vector<char> bytes;

	switch (type)
	{
		case TypeEntity::Boolean:
			bytes.push_back((char)Command::bastore);
			break;

		case TypeEntity::UntypedInt:
		case TypeEntity::Int:
			bytes.push_back((char)Command::iastore);
			break;

		case TypeEntity::UntypedFloat:
		case TypeEntity::Float:
			bytes.push_back((char)Command::fastore);
			break;

		case TypeEntity::Array:
		case TypeEntity::String:
			bytes.push_back((char)Command::aastore);
			break;

		default:
			break;
	}

	return bytes;
}

std::vector<char> Generator::generateLoadFromArrayCommand(TypeEntity::TypeEntityEnum type) {
	std::vector<char> bytes;

	switch (type)
	{
		case TypeEntity::Boolean:
			bytes.push_back((char)Command::baload);
			break;

		case TypeEntity::UntypedInt:
		case TypeEntity::Int:
			bytes.push_back((char)Command::iaload);
			break;

		case TypeEntity::UntypedFloat:
		case TypeEntity::Float:
			bytes.push_back((char)Command::faload);
			break;

		case TypeEntity::Array:
		case TypeEntity::String:
			bytes.push_back((char)Command::aaload);
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
		return generate(keywordStatement);

	} else if (auto assignmentStatement = dynamic_cast<AssignmentStatement*>(stmt)) {
		return generate(assignmentStatement);

	} else if (auto whileStatement = dynamic_cast<WhileStatement*>(stmt)) {
		return generate(whileStatement);

	} else if (auto forStatement = dynamic_cast<ForStatement*>(stmt)) {
		std::cout << "For loop is not exists";

	} else if (auto forRangeStatement = dynamic_cast<ForRangeStatement*>(stmt)) {
		std::cout << "For range loop is not exists";

	} else if (auto ifStatement = dynamic_cast<IfStatement*>(stmt)) {
		return generate(ifStatement);

	} else if (auto switchCaseClause = dynamic_cast<SwitchCaseClause*>(stmt)) {
		std::cout << "Switch case clause is non-callable for generate";

	} else if (auto switchStatement = dynamic_cast<SwitchStatement*>(stmt)) {
		return generate(switchStatement);
	}

	return {};
};

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
