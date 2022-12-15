#pragma once
#include "../visitor.h"

#include <iostream>

class DotConvertVisitor : public Visitor {
    std::ostream& out;

    static std::string MakeNode(std::size_t id, std::string_view name);
    static std::string MakeConnection(size_t id1, size_t id2, std::string_view note = "");

    void visit(const NodeAST* node) override;
    void visit(const PackageAST* node) override;
    void visit(const VariableDeclaration* node) override;
    void visit(const TypeDeclaration* node) override;
    void visit(const FunctionDeclaration* node) override;
    void visit(const MethodDeclaration* node) override;
    void visit(const IdentifierAsExpression* node) override;
    void visit(const IntegerExpression* node) override;
    void visit(const BooleanExpression* node) override;
    void visit(const FloatExpression* node) override;
    void visit(const StringExpression* node) override;
    void visit(const RuneExpression* node) override;
    void visit(const NilExpression* node) override;
    void visit(const FunctionLitExpression* node) override;
    void visit(const UnaryExpression* node) override;
    void visit(const BinaryExpression* node) override;
    void visit(const CallableExpression* node) override;
    void visit(const AccessExpression* node) override;
    void visit(const BlockStatement* node) override;
    void visit(const KeywordStatement* node) override;
    void visit(const ExpressionStatement* node) override;
    void visit(const AssignmentStatement* node) override;
    void visit(const ForStatement* node) override;
    void visit(const WhileStatement* node) override;
    void visit(const ForRangeStatement* node) override;
    void visit(const ReturnStatement* node) override;
    void visit(const IfStatement* node) override;
    void visit(const SwitchCaseClause* node) override;
    void visit(const SwitchStatement* node) override;
    void visit(const DeclarationStatement* node) override;
    void visit(const IdentifiersWithType* node) override;
    void visit(const FunctionSignature* node) override;
    void visit(const ArraySignature* node) override;
    void visit(const StructSignature* node) override;
    void visit(const IdentifierAsType* node) override;
    void visit(const InterfaceType* node) override;
    void visit(const CompositeLiteral* node) override;
    void visit(const ElementCompositeLiteral* node) override;

public:
    explicit DotConvertVisitor(std::ostream &out) : out(out) {};
    void convert(NodeAST* node);
    ~DotConvertVisitor() override = default;
};

