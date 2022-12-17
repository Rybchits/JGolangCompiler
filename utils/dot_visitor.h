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

    void visit(NodeAST* node) override;
    void visit(PackageAST* node) override;
    void visit(VariableDeclaration* node) override;
    void visit(TypeDeclaration* node) override;
    void visit(FunctionDeclaration* node) override;
    void visit(MethodDeclaration* node) override;
    void visit(IdentifierAsExpression* node) override;
    void visit(IntegerExpression* node) override;
    void visit(BooleanExpression* node) override;
    void visit(FloatExpression* node) override;
    void visit(StringExpression* node) override;
    void visit(RuneExpression* node) override;
    void visit(NilExpression* node) override;
    void visit(FunctionLitExpression* node) override;
    void visit(UnaryExpression* node) override;
    void visit(BinaryExpression* node) override;
    void visit(CallableExpression* node) override;
    void visit(AccessExpression* node) override;
    void visit(BlockStatement* node) override;
    void visit(KeywordStatement* node) override;
    void visit(ExpressionStatement* node) override;
    void visit(AssignmentStatement* node) override;
    void visit(ForStatement* node) override;
    void visit(WhileStatement* node) override;
    void visit(ForRangeStatement* node) override;
    void visit(ReturnStatement* node) override;
    void visit(IfStatement* node) override;
    void visit(SwitchCaseClause* node) override;
    void visit(SwitchStatement* node) override;
    void visit(DeclarationStatement* node) override;
    void visit(IdentifiersWithType* node) override;
    void visit(FunctionSignature* node) override;
    void visit(ArraySignature* node) override;
    void visit(StructSignature* node) override;
    void visit(IdentifierAsType* node) override;
    void visit(InterfaceType* node) override;
    void visit(CompositeLiteral* node) override;
    void visit(ElementCompositeLiteral* node) override;

public:
    explicit DotConvertVisitor(std::ostream &out) : out(out) {};
    void convert(NodeAST* node);
    ~DotConvertVisitor() override = default;
};

