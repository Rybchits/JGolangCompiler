#pragma once

#include "../context.h"
#include "./semantic.h"

#include <vector>

class TypesVisitor;
class Semantic;

class VariableEntity{
public:
    TypeEntity* type;
    bool isConst;
    bool isArgument;
    int numberUsage = 0;

    VariableEntity(TypeEntity* type, bool isConst = false, bool isArgument = false): 
        type(type), isConst(isConst), isArgument(isArgument) {};

    void use() { numberUsage++; };
};


class ConstExpressionVisitor : private Visitor {
private:
    TypesVisitor* typesVisitor;
    bool constValid = true;

    void onFinishVisit(IdentifierAsExpression* node);
    void onFinishVisit(CallableExpression* node);
    void onFinishVisit(AccessExpression* node);
    void onFinishVisit(CompositeLiteral* node);

public:
    bool isConstExpression(ExpressionAST* expr);
    ConstExpressionVisitor(TypesVisitor* visitor): typesVisitor(visitor) {};
};


class TypesVisitor : public Visitor {
friend class ConstExpressionVisitor;
private:
    Semantic* semantic;
    Context<VariableEntity*> scopesDeclarations;

    ConstExpressionVisitor constCheckVisitor = ConstExpressionVisitor(this);

    MethodEntity* currentMethodEntity;
    int numberLocalVariables = 0;

    // supporting 
    bool lastAddedScopeInFuncDecl = false;

    TypeEntity* typeCurrentArray;
    int indexCurrentAxisArray;

    void onStartVisit(CompositeLiteral* node);
    void onStartVisit(ElementCompositeLiteral* node);
    void onStartVisit(BlockStatement* node);
    void onStartVisit(ExpressionStatement* node);

    void onFinishVisit(BlockStatement* node);
    void onFinishVisit(VariableDeclaration* node);
    void onFinishVisit(ShortVarDeclarationStatement* node);
    void onFinishVisit(AssignmentStatement* node);
    void onFinishVisit(ReturnStatement* node);
    void onFinishVisit(WhileStatement* node);
    void onFinishVisit(IfStatement* node);
    void onFinishVisit(SwitchStatement* node);

    void onFinishVisit(IdentifierAsExpression* node);
    void onFinishVisit(IntegerExpression* node);
    void onFinishVisit(BooleanExpression* node);
    void onFinishVisit(FloatExpression* node);
    void onFinishVisit(StringExpression* node);
    void onFinishVisit(NilExpression* node);
    void onFinishVisit(UnaryExpression* node);
    void onFinishVisit(BinaryExpression* node);
    void onFinishVisit(CallableExpression* node);
    void onFinishVisit(AccessExpression* node);
    void onFinishVisit(CompositeLiteral* node);
    void onFinishVisit(ElementCompositeLiteral* node);

    bool defineTypeBuiltInFunction(CallableExpression* function);
    bool definePrintsFunctions(CallableExpression* function);
    bool defineLenFunction(CallableExpression* function);
    bool defineAppendFunction(CallableExpression* function);
    bool defineReadFunction(CallableExpression* function, TypeEntity::TypeEntityEnum type);

public:
    explicit TypesVisitor(Semantic* semantic): semantic(semantic) {};
    void analyzePackageClass(ClassEntity* classEntity, std::vector<std::string>& idsConstPackageVariables);
};