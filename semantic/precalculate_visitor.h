#pragma once

#include "../visitor.h"
#include "./semantic.h"

class PrecalculateVisitor : public Visitor {
private:
    Semantic* semantic;

    ExpressionAST* transformExpression(ExpressionAST* expr);
    ExpressionList transformExpressionList(ExpressionList& list);

    ExpressionAST* transformUnaryExpression(UnaryExpression* expr);
    ExpressionAST* transformBinaryExpression(BinaryExpression* expr);

    void onFinishVisit(VariableDeclaration* node) override;
    void onFinishVisit(UnaryExpression* node) override;
    void onFinishVisit(BinaryExpression* node) override;
    void onFinishVisit(CallableExpression* node) override;
    void onFinishVisit(AccessExpression* node) override;
    void onFinishVisit(ElementCompositeLiteral* node) override;
    void onFinishVisit(ExpressionStatement* node) override;
    void onFinishVisit(ReturnStatement* node) override;
    void onFinishVisit(AssignmentStatement* node) override;
    void onFinishVisit(ForStatement* node) override;
    void onFinishVisit(ShortVarDeclarationStatement* node) override;
    void onFinishVisit(WhileStatement* node) override;
    void onFinishVisit(IfStatement* node) override;
    void onFinishVisit(SwitchStatement* node) override;
    void onFinishVisit(SwitchCaseClause* node) override;

public:
    void transform(PackageAST* packageAst);
    explicit PrecalculateVisitor(Semantic* semantic): semantic(semantic) {};
    ~PrecalculateVisitor() override = default;
};

