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
    virtual void visit(NodeAST* node) = 0;
    virtual void visit(PackageAST* node) = 0;
    virtual void visit(VariableDeclaration* node) = 0;
    virtual void visit(TypeDeclaration* node) = 0;
    virtual void visit(FunctionDeclaration* node) = 0;
    virtual void visit(MethodDeclaration* node) = 0;
    virtual void visit(IdentifierAsExpression* node) = 0;
    virtual void visit(BooleanExpression* node) = 0;
    virtual void visit(FloatExpression* node) = 0;
    virtual void visit(StringExpression* node) = 0;
    virtual void visit(RuneExpression* node) = 0;
    virtual void visit(NilExpression* node) = 0;
    virtual void visit(FunctionLitExpression* node) = 0;
    virtual void visit(UnaryExpression* node) = 0;
    virtual void visit(BinaryExpression* node) = 0;
    virtual void visit(CallableExpression* node) = 0;
    virtual void visit(AccessExpression* node) = 0;
    virtual void visit(BlockStatement* node) = 0;
    virtual void visit(KeywordStatement* node) = 0;
    virtual void visit(ExpressionStatement* node) = 0;
    virtual void visit(AssignmentStatement* node) = 0;
    virtual void visit(ForStatement* node) = 0;
    virtual void visit(WhileStatement* node) = 0;
    virtual void visit(ForRangeStatement* node) = 0;
    virtual void visit(ReturnStatement* node) = 0;
    virtual void visit(IfStatement* node) = 0;
    virtual void visit(SwitchCaseClause* node) = 0;
    virtual void visit(SwitchStatement* node) = 0;
    virtual void visit(DeclarationStatement* node) = 0;
    virtual void visit(IdentifiersWithType* node) = 0;
    virtual void visit(FunctionSignature* node) = 0;
    virtual void visit(ArraySignature* node) = 0;
    virtual void visit(StructSignature* node) = 0;
    virtual void visit(IdentifierAsType* node) = 0;
    virtual void visit(InterfaceType* node) = 0;
    virtual void visit(CompositeLiteral* node) = 0;
    virtual void visit(ElementCompositeLiteral* node) = 0;
    virtual void visit(IntegerExpression* node) = 0;
    virtual ~Visitor() = default;
};


// thin/empty implementation
// not inheriting std exceptions
// and not inheritable
struct ProcessingInterrupted final {};