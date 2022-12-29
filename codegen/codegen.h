#pragma once

#include "./constant.h"
#include "./commands.h"
#include "./constant_visitor.h"
#include "../visitor.h"
#include "../semantic/entities.h"
#include "../semantic/semantic.h"

#include <unordered_map>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

struct RefConstant {
    bool isLocal;
    int index;
    RefConstant(int index, bool isLocalVariable): index(index), isLocal(isLocalVariable) {};
};

class ContextGenerator {
private:
    std::vector<std::unordered_map<std::string, RefConstant*>> scopes;

public:
    void addScope() { scopes.push_back(std::unordered_map<std::string, RefConstant*>()); };
    void popScope() { scopes.pop_back(); }

    bool addConstant(std::string id, RefConstant* constant) { 
        return scopes.back()[id] = constant; 
    }

    RefConstant* findConstant(std::string name) {
        for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
            if ((*scope).count(name)) 
                return (*scope)[name];
        }

        return nullptr;
    };

    ContextGenerator() { addScope(); };
};

class Generator {
    std::unordered_map<size_t, TypeEntity*> typesExpressions;
    std::unordered_map<std::string, ClassEntity*> classes;

    ConstantPool constantPool;
    ContextGenerator context;
    std::fstream outfile;

    MethodEntity* currentMethod;
    int indexCurrentLocalVariable = 0;

    void generateConstant(Constant & constant);

    void generateField(std::string fieldName, FieldEntity* field);
    void generateMethod(std::string_view methodName, std::string_view descriptor
                , uint numeberLocals, uint accessFlags
                , std::vector<char>&& bodyCodeBytes);

    std::vector<char> generateGlobalClassConstructorCode();
    std::vector<char> generateStaticConstuctorCode(std::string_view className, ClassEntity* classEntity);
    std::vector<char> generateMethodBodyCode(MethodEntity* methodEntity);

    std::vector<char> generate(BlockStatement* block);

    std::vector<char> generate(ReturnStatement* stmt);
    std::vector<char> generate(ExpressionStatement* stmt);
    std::vector<char> generate(DeclarationStatement* stmt);

    std::vector<char> generate(CallableExpression* expr);
    std::vector<char> generate(IdentifierAsExpression* expr);
    std::vector<char> generate(StringExpression* expr);
    std::vector<char> generate(IntegerExpression* expr);
    std::vector<char> generate(FloatExpression* expr);
    std::vector<char> generate(BooleanExpression* expr);
    std::vector<char> generate(UnaryExpression* expr);

    std::vector<char> generate(ExpressionAST* expr);
    std::vector<char> generate(StatementAST* stmt);

    void fillConstantPool(std::string_view className, ClassEntity* classEntity);
    void addBuiltInFunctions(std::string_view nameBaseClass, const std::unordered_map<std::string, TypeEntity*>& functions);

public:
    Generator(std::unordered_map<std::string, ClassEntity*>& classPool
                , std::unordered_map<size_t, TypeEntity*>& typesExprs): classes(classPool), typesExpressions(typesExprs) {};
    
    void generate();
};
