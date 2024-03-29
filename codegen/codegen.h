#pragma once

#include "./constant.h"
#include "./commands.h"
#include "../visitor.h"
#include "../context.h"
#include "../semantic/semantic.h"

#include <unordered_map>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <variant>
#include <algorithm>
#include <numeric>

struct RefConstant {
    bool isLocal;
    int index;
    RefConstant(int index, bool isLocalVariable): index(index), isLocal(isLocalVariable) {};
};

class Generator {
private:
    std::unordered_map<std::string, ClassEntity*> classes;

    ConstantPool constantPool;
    Context<RefConstant*> context;

    MethodEntity* currentMethod;
    int indexCurrentLocalVariable = 0;

    const int8_t BREAK_FILLER = 0xFB;
    const int8_t CONTINUE_FILLER = 0xFC;

    std::vector<char> generateInteger(int64_t number);
    std::vector<char> generateFloating(float number);

    std::vector<char> generateNewArray(ArraySignatureEntity* arrayType, ElementCompositeLiteralList elements);
    std::vector<char> generateNewArrayCommand(TypeEntity* elementType);

    std::vector<char> generateField(std::string fieldName, FieldEntity* field);
    std::vector<char> generateMethod(std::string_view methodName, std::string_view descriptor
                , uint numeberLocals, uint accessFlags
                , std::vector<char>&& bodyCodeBytes);

    std::vector<char> generateGlobalClassConstructorCode();
    std::vector<char> generateStaticConstuctorCode(std::string_view className, ClassEntity* classEntity);
    std::vector<char> generateMethodBodyCode(MethodEntity* methodEntity);

    std::vector<char> generate(BlockStatement* block);
    std::vector<char> generate(ReturnStatement* stmt);
    std::vector<char> generate(ExpressionStatement* stmt);
    std::vector<char> generate(DeclarationStatement* stmt);
    std::vector<char> generate(ShortVarDeclarationStatement* stmt);
    std::vector<char> generate(AssignmentStatement* stmt);
    std::vector<char> generate(IfStatement* stmt);
    std::vector<char> generate(WhileStatement* stmt);
    std::vector<char> generate(SwitchStatement* stmt);
    std::vector<char> generate(KeywordStatement* stmt);

    std::vector<char> generate(CallableExpression* expr);
    std::vector<char> generate(IdentifierAsExpression* expr);
    std::vector<char> generate(StringExpression* expr);
    std::vector<char> generate(IntegerExpression* expr);
    std::vector<char> generate(FloatExpression* expr);
    std::vector<char> generate(BooleanExpression* expr);
    std::vector<char> generate(UnaryExpression* expr);
    std::vector<char> generate(BinaryExpression* expr);
    std::vector<char> generate(AccessExpression* expr);

    std::vector<char> generate(CompositeLiteral* expr);
    std::vector<char> generate(ElementCompositeLiteral* expr);

    std::vector<char> generate(ExpressionAST* expr);
    std::vector<char> generate(StatementAST* stmt);

    std::vector<char> generateStoreToVariableCommand(std::string variableIdentifier, TypeEntity::TypeEntityEnum type);
    std::vector<char> generateStoreToArrayCommand(TypeEntity::TypeEntityEnum type);
    std::vector<char> generateLoadFromArrayCommand(TypeEntity::TypeEntityEnum type);
    std::vector<char> generateCloneArrayCommand(ExpressionAST* array);

    void replaceBreakFillers(std::vector<char>& bytes);
    void replaceContinueFillers(std::vector<char>& bytes);

    std::vector<char> initializeLocalVariables(const IdentifiersList& identifiers, const ExpressionList& values);
    void fillConstantPool(std::string_view className, ClassEntity* classEntity);

    std::string createDescriptorBuiltInFunction(CallableExpression* expr);

public:
    Generator(std::unordered_map<std::string, ClassEntity*>& classPool): classes(classPool) {};
    
    void generate();
};
