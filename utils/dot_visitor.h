#pragma once
#include "../visitor.h"

#include <iostream>
#include <unordered_map>
#include <vector>

class DotConvertVisitor : public Visitor {
    std::ostream& out;
    std::unordered_map<std::size_t, std::vector<std::size_t>> connections;

    bool hasConnection(size_t id1, size_t id2);
    std::string MakeNode(std::size_t id, std::string_view name);
    std::string MakeConnection(size_t id1, size_t id2, std::string_view note = "");

    void onStartVisit(NodeAST* node) override;
    void onStartVisit(PackageAST* node) override;
    void onStartVisit(VariableDeclaration* node) override;
    void onStartVisit(TypeDeclaration* node) override;
    void onStartVisit(FunctionDeclaration* node) override;
    void onStartVisit(MethodDeclaration* node) override;
    void onStartVisit(IdentifierAsExpression* node) override;
    void onStartVisit(IntegerExpression* node) override;
    void onStartVisit(BooleanExpression* node) override;
    void onStartVisit(FloatExpression* node) override;
    void onStartVisit(StringExpression* node) override;
    void onStartVisit(NilExpression* node) override;
    void onStartVisit(FunctionLitExpression* node) override;
    void onStartVisit(UnaryExpression* node) override;
    void onStartVisit(BinaryExpression* node) override;
    void onStartVisit(CallableExpression* node) override;
    void onStartVisit(AccessExpression* node) override;
    void onStartVisit(BlockStatement* node) override;
    void onStartVisit(KeywordStatement* node) override;
    void onStartVisit(ExpressionStatement* node) override;
    void onStartVisit(AssignmentStatement* node) override;
    void onStartVisit(ForStatement* node) override;
    void onStartVisit(WhileStatement* node) override;
    void onStartVisit(ForRangeStatement* node) override;
    void onStartVisit(ReturnStatement* node) override;
    void onStartVisit(IfStatement* node) override;
    void onStartVisit(SwitchCaseClause* node) override;
    void onStartVisit(SwitchStatement* node) override;
    void onStartVisit(DeclarationStatement* node) override;
    void onStartVisit(IdentifiersWithType* node) override;
    void onStartVisit(FunctionSignature* node) override;
    void onStartVisit(ArraySignature* node) override;
    void onStartVisit(StructSignature* node) override;
    void onStartVisit(IdentifierAsType* node) override;
    void onStartVisit(InterfaceType* node) override;
    void onStartVisit(CompositeLiteral* node) override;
    void onStartVisit(ElementCompositeLiteral* node) override;
    void onStartVisit(ShortVarDeclarationStatement *node) override;

public:
    explicit DotConvertVisitor(std::ostream &out) : out(out) {};
    void convert(NodeAST* node);
    ~DotConvertVisitor() override = default;
};

