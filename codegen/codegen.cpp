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


void Generator::addBuiltInFunctions(std::string_view nameBaseClass, const std::unordered_map<std::string, TypeEntity*>& functions) {
	for (auto &[id, descriptor] : functions) {
		int index = constantPool.FindMethodRef(nameBaseClass, id, descriptor->toByteCode());
		context.addConstant(id, new RefConstant(index, false));
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

void Generator::generate() {
    using namespace std::filesystem;
	create_directory(current_path() / "output");
	std::vector<char> buffer;

    for (auto & [className, classEntity] : classes) {
		constantPool = ConstantPool();
		context = ContextGenerator();
		
		fillConstantPool(className, classEntity);
		addBuiltInFunctions("$Base", Semantic::BuiltInFunctions);

        // Create class file
        const auto filename = std::string{ className } + ".class";

        auto filepath = current_path() / "output" / filename;
		
        outfile.open(filepath, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);

		// CAFEBABE
		outfile << (char)0xCA << (char)0xFE << (char)0xBA << (char)0xBE;

		// JAVA 8 (version 52.0 (0x34))
		outfile << (char)0x00 << (char)0x00 << (char)0x00 << (char)0x3E;

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
		buffer = intToBytes(constantPool.FindClass(className));
		outfile << buffer[2] << buffer[3];

		// Parent class constant
		buffer = intToBytes(constantPool.FindClass("java/lang/Object"));
		outfile << buffer[2] << buffer[3];

		// Interfaces table
		outfile << (char)0x00 << (char)0x00;

		// fields count
		buffer = intToBytes(classEntity->getFields().size());
		outfile << buffer[2] << buffer[3];
		
		// fields info
		for (auto & [fieldIdentifier, fieldEntity] : classEntity->getFields()) {
			generateField(fieldIdentifier, fieldEntity);
		}

		bool hasClassInitializationConstructor = classEntity->hasFieldsDeclaration();

		// methods count
		buffer = intToBytes(classEntity->getMethods().size() + 1 + hasClassInitializationConstructor);
		outfile << buffer[2] << buffer[3];

		// Generate constructor
		generateMethod("<init>", "()V", 1, uint16_t(AccessFlags::Public),
						generateGlobalClassConstructorCode());

		if (hasClassInitializationConstructor) {
			generateMethod("<clinit>", "()V", 0, uint16_t(AccessFlags::Static)
						  , generateStaticConstuctorCode(className, classEntity));
		}

		for (auto & [methodIdentifier, methodEntity] : classEntity->getMethods()) {
			generateMethod(methodIdentifier, methodEntity->toTypeEntity()->toByteCode()
						 , methodEntity->getNumberLocalVariables() + 1, uint16_t(AccessFlags::Public)
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

std::vector<char> Generator::generateStaticConstuctorCode(std::string_view className, ClassEntity* classEntity) {
	std::vector<char> bytes;

	for (auto & [fieldIdentifier, field] : classEntity->getFields()) {
		if (!field->hasDeclaration()) continue;

		auto buffer = generate(field->declaration);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		bytes.push_back(char(Command::putstatic));

		buffer = intToBytes(constantPool.FindFieldRef(className, fieldIdentifier, field->type->toByteCode()));
		bytes.push_back(buffer[2]);
		bytes.push_back(buffer[3]);
	}

	bytes.push_back(char(Command::return_));
	return bytes;
}

std::vector<char> Generator::generateMethodBodyCode(MethodEntity* methodEntity) {
	context.addScope();
	indexCurrentLocalVariable = 1;
	currentMethod = methodEntity;

	for (auto &[name, _] : methodEntity->getArguments()) {
		context.addConstant(name, new RefConstant(indexCurrentLocalVariable++, true));
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

	context.addScope();

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

	auto refConst = context.findConstant(expr->identifier);
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
		codeBytes.push_back(buffer[2]);
		codeBytes.push_back(buffer[3]);
	}

	return codeBytes;
}

std::vector<char> Generator::generate(StringExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes.push_back((char)Command::ldc_w);
	buffer = intToBytes(constantPool.FindString(expr->stringLit));
	codeBytes.push_back(buffer[2]);
	codeBytes.push_back(buffer[3]);

	return codeBytes;
}

std::vector<char> Generator::generate(IntegerExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	if (expr->intLit > -1 && expr->intLit < 6) {
		codeBytes.push_back(char(Command::iconst_0) + expr->intLit);

	} else {
		codeBytes.push_back((char)Command::ldc_w);
		buffer = intToBytes(constantPool.FindInt(expr->intLit));
		codeBytes.push_back(buffer[2]);
		codeBytes.push_back(buffer[3]);
	}
	
	return codeBytes;
}


std::vector<char> Generator::generate(FloatExpression* expr) {
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes.push_back((char)Command::ldc_w);
	buffer = intToBytes(constantPool.FindFloat(expr->floatLit));
	codeBytes.push_back(buffer[2]);
	codeBytes.push_back(buffer[3]);

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


// TODO here
std::vector<char> Generator::generate(UnaryExpression* expr) {	
	std::vector<char> codeBytes;
	std::vector<char> buffer;

	codeBytes = generate(expr->expression);

	switch (expr->type)
	{
	case UnaryExpressionEnum::UnaryMinus:
		codeBytes.push_back(char(Command::ineg));
		break;

	case UnaryExpressionEnum::Increment:
		// codeBytes.push_back(char(Command))
		break;

	case UnaryExpressionEnum::Decrement:
			codeBytes.push_back(char(Command::iinc));
		break;

	case UnaryExpressionEnum::UnaryNot:
		/* code */
		break;
	
	default:
		break;
	}
	
	return codeBytes;
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
				context.addConstant(id, new RefConstant(indexCurrentLocalVariable++, true));
			}

			buffer = initializeLocalVariables(varDecl->identifiersWithType->identifiers, varDecl->values);
			bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		}
	}
	
	return bytes;
}

std::vector<char> Generator::generate(ShortVarDeclarationStatement* stmt) {
	for (auto id : stmt->identifiers) {
		context.addConstant(id, new RefConstant(indexCurrentLocalVariable++, true));
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
		std::cout << "не сделали rune expression";
		
	} else if (auto unaryExpression = dynamic_cast<UnaryExpression*>(expr)) {
		std::cout << "не сделали unary expression";

	} else if (auto binaryExpression = dynamic_cast<BinaryExpression*>(expr)) {
		std::cout << "не сделали binary expression";

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
				// right aastore index left

				//buffer = generate((*rightIterator));
				//bytes.insert(bytes.end(), buffer.begin(), buffer.end());
				
			} else if (auto identifierAsExpression = dynamic_cast<IdentifierAsExpression*>(*leftIterator)){
				buffer = generate(*rightIterator);
				bytes.insert(bytes.end(), buffer.begin(), buffer.end());

				auto constantRef = context.findConstant(identifierAsExpression->identifier);

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
			buffer = intToBytes(context.findConstant(variableIdentifier)->index);
			bytes.push_back(buffer[3]);
			break;
		case TypeEntity::String:
			bytes.push_back((char)Command::astore);
			buffer = intToBytes(context.findConstant(variableIdentifier)->index);
			bytes.push_back(buffer[3]);
			break;

		case TypeEntity::UntypedFloat:
		case TypeEntity::Float:
			bytes.push_back((char)Command::fstore);
			buffer = intToBytes(context.findConstant(variableIdentifier)->index);
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