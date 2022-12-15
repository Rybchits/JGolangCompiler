#pragma once

#include "ast.h"

class NodeAST;
class PackageAST;
class VariableDeclaration;
class TypeDeclaration;
class FunctionDeclaration;
class MethodDeclaration;
class IdentifierAsExpression;
class BooleanExpression;
class FloatExpression;
class StringExpression;
class RuneExpression;
class NilExpression;
class FunctionLitExpression;
class UnaryExpression;
class BinaryExpression;
class CallableExpression;
class AccessExpression;
class BlockStatement;
class KeywordStatement;
class ExpressionStatement;
class AssignmentStatement;
class ForStatement;
class WhileStatement;
class ForRangeStatement;
class ReturnStatement;
class IfStatement;
class SwitchCaseClause;
class SwitchStatement;
class DeclarationStatement;
class IdentifiersWithType;
class FunctionSignature;
class ArraySignature;
class StructSignature;
class IdentifierAsType;
class InterfaceType;
class CompositeLiteral;
class ElementCompositeLiteral;
class IntegerExpression;

class Visitor {
public:
    virtual void visit(const NodeAST* node) = 0;
    virtual void visit(const PackageAST* node) = 0;
    virtual void visit(const VariableDeclaration* node) = 0;
    virtual void visit(const TypeDeclaration* node) = 0;
    virtual void visit(const FunctionDeclaration* node) = 0;
    virtual void visit(const MethodDeclaration* node) = 0;
    virtual void visit(const IdentifierAsExpression* node) = 0;
    virtual void visit(const BooleanExpression* node) = 0;
    virtual void visit(const FloatExpression* node) = 0;
    virtual void visit(const StringExpression* node) = 0;
    virtual void visit(const RuneExpression* node) = 0;
    virtual void visit(const NilExpression* node) = 0;
    virtual void visit(const FunctionLitExpression* node) = 0;
    virtual void visit(const UnaryExpression* node) = 0;
    virtual void visit(const BinaryExpression* node) = 0;
    virtual void visit(const CallableExpression* node) = 0;
    virtual void visit(const AccessExpression* node) = 0;
    virtual void visit(const BlockStatement* node) = 0;
    virtual void visit(const KeywordStatement* node) = 0;
    virtual void visit(const ExpressionStatement* node) = 0;
    virtual void visit(const AssignmentStatement* node) = 0;
    virtual void visit(const ForStatement* node) = 0;
    virtual void visit(const WhileStatement* node) = 0;
    virtual void visit(const ForRangeStatement* node) = 0;
    virtual void visit(const ReturnStatement* node) = 0;
    virtual void visit(const IfStatement* node) = 0;
    virtual void visit(const SwitchCaseClause* node) = 0;
    virtual void visit(const SwitchStatement* node) = 0;
    virtual void visit(const DeclarationStatement* node) = 0;
    virtual void visit(const IdentifiersWithType* node) = 0;
    virtual void visit(const FunctionSignature* node) = 0;
    virtual void visit(const ArraySignature* node) = 0;
    virtual void visit(const StructSignature* node) = 0;
    virtual void visit(const IdentifierAsType* node) = 0;
    virtual void visit(const InterfaceType* node) = 0;
    virtual void visit(const CompositeLiteral* node) = 0;
    virtual void visit(const ElementCompositeLiteral* node) = 0;
    virtual void visit(const IntegerExpression* node) = 0;
    virtual ~Visitor() = default;;
};


// thin/empty implementation
// not inheriting std exceptions
// and not inheritable
struct ProcessingInterrupted final {};