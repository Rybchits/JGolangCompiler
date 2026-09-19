#pragma once

#include "../context.h"
#include "../entities/class_entity.h"
#include "../visitor.h"

#include <vector>

class TypesVisitor;

class VariableEntity{
public:
    TypePtr type;
    bool isConst;
    bool isArgument;
    int numberUsage = 0;

    VariableEntity(TypePtr type, bool isConst = false, bool isArgument = false):
        type(std::move(type)), isConst(isConst), isArgument(isArgument) {};

    void use() { numberUsage++; };
};


class ConstExpressionVisitor : private Visitor {
private:
    TypesVisitor* typesVisitor;
    bool constValid = true;

    void onFinishVisit(IdentifierAsExpression& node) override;
    void onFinishVisit(CallableExpression& node) override;
    void onFinishVisit(AccessExpression& node) override;
    void onFinishVisit(CompositeLiteral& node) override;

public:
    bool isConstExpression(ExpressionAST* expr);
    ConstExpressionVisitor(TypesVisitor* visitor): typesVisitor(visitor) {};
};


class TypesVisitor : public Visitor {
friend class ConstExpressionVisitor;
private:
    std::vector<std::string> errors;

    void addError(const std::string& message) { errors.push_back(message); }
    Context<VariableEntity> scopesDeclarations;

    ConstExpressionVisitor constCheckVisitor = ConstExpressionVisitor(this);

    MethodEntity* currentMethodEntity;
    int numberLocalVariables = 0;

    // supporting 
    bool lastAddedScopeInFuncDecl = false;

    TypePtr typeCurrentArray;
    int indexCurrentAxisArray;

    void onStartVisit(CompositeLiteral& node) override;
    void onStartVisit(ElementCompositeLiteral& node) override;
    void onStartVisit(BlockStatement& node) override;
    void onStartVisit(ExpressionStatement& node) override;

    void onFinishVisit(BlockStatement& node) override;
    void onFinishVisit(VariableDeclaration& node) override;
    void onFinishVisit(ShortVarDeclarationStatement& node) override;
    void onFinishVisit(AssignmentStatement& node) override;
    void onFinishVisit(ReturnStatement& node) override;
    void onFinishVisit(WhileStatement& node) override;
    void onFinishVisit(IfStatement& node) override;
    void onFinishVisit(SwitchStatement& node) override;

    void onFinishVisit(IdentifierAsExpression& node) override;
    void onFinishVisit(IntegerExpression& node) override;
    void onFinishVisit(BooleanExpression& node) override;
    void onFinishVisit(FloatExpression& node) override;
    void onFinishVisit(StringExpression& node) override;
    void onFinishVisit(NilExpression& node) override;
    void onFinishVisit(UnaryExpression& node) override;
    void onFinishVisit(BinaryExpression& node) override;
    void onFinishVisit(CallableExpression& node) override;
    void onFinishVisit(AccessExpression& node) override;
    void onFinishVisit(CompositeLiteral& node) override;
    void onFinishVisit(ElementCompositeLiteral& node) override;

    bool defineTypeBuiltInFunction(CallableExpression* function);
    bool definePrintsFunctions(CallableExpression* function);
    bool defineLenFunction(CallableExpression* function);
    bool defineAppendFunction(CallableExpression* function);
    bool defineReadFunction(CallableExpression* function, TypeEntity::TypeEntityEnum type);

public:
    const std::vector<std::string>& getErrors() const { return errors; }
    void analyzePackageClass(ClassEntity* classEntity, std::vector<std::string>& idsConstPackageVariables);
};