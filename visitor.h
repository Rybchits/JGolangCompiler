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
    virtual void onStartVisit(NodeAST* node) {};
    virtual void onStartVisit(PackageAST* node) {};
    virtual void onStartVisit(VariableDeclaration* node) {};
    virtual void onStartVisit(TypeDeclaration* node) {};
    virtual void onStartVisit(FunctionDeclaration* node) {};
    virtual void onStartVisit(MethodDeclaration* node) {};
    virtual void onStartVisit(IdentifierAsExpression* node) {};
    virtual void onStartVisit(BooleanExpression* node) {};
    virtual void onStartVisit(FloatExpression* node) {};
    virtual void onStartVisit(StringExpression* node) {};
    virtual void onStartVisit(RuneExpression* node) {};
    virtual void onStartVisit(NilExpression* node) {};
    virtual void onStartVisit(FunctionLitExpression* node) {};
    virtual void onStartVisit(UnaryExpression* node) {};
    virtual void onStartVisit(BinaryExpression* node) {};
    virtual void onStartVisit(CallableExpression* node) {};
    virtual void onStartVisit(AccessExpression* node) {};
    virtual void onStartVisit(BlockStatement* node) {};
    virtual void onStartVisit(KeywordStatement* node) {};
    virtual void onStartVisit(ExpressionStatement* node) {};
    virtual void onStartVisit(AssignmentStatement* node) {};
    virtual void onStartVisit(ForStatement* node) {};
    virtual void onStartVisit(WhileStatement* node) {};
    virtual void onStartVisit(ForRangeStatement* node) {};
    virtual void onStartVisit(ReturnStatement* node) {};
    virtual void onStartVisit(IfStatement* node) {};
    virtual void onStartVisit(SwitchCaseClause* node) {};
    virtual void onStartVisit(SwitchStatement* node) {};
    virtual void onStartVisit(DeclarationStatement* node) {};
    virtual void onStartVisit(IdentifiersWithType* node) {};
    virtual void onStartVisit(FunctionSignature* node) {};
    virtual void onStartVisit(ArraySignature* node) {};
    virtual void onStartVisit(StructSignature* node) {};
    virtual void onStartVisit(IdentifierAsType* node) {};
    virtual void onStartVisit(InterfaceType* node) {};
    virtual void onStartVisit(CompositeLiteral* node) {};
    virtual void onStartVisit(ElementCompositeLiteral* node) {};
    virtual void onStartVisit(IntegerExpression* node) {};

    virtual void onFinishVisit(NodeAST* node) {};
    virtual void onFinishVisit(PackageAST* node) {};
    virtual void onFinishVisit(VariableDeclaration* node) {};
    virtual void onFinishVisit(TypeDeclaration* node) {};
    virtual void onFinishVisit(FunctionDeclaration* node) {};
    virtual void onFinishVisit(MethodDeclaration* node) {};
    virtual void onFinishVisit(IdentifierAsExpression* node) {};
    virtual void onFinishVisit(BooleanExpression* node) {};
    virtual void onFinishVisit(FloatExpression* node) {};
    virtual void onFinishVisit(StringExpression* node) {};
    virtual void onFinishVisit(RuneExpression* node) {};
    virtual void onFinishVisit(NilExpression* node) {};
    virtual void onFinishVisit(FunctionLitExpression* node) {};
    virtual void onFinishVisit(UnaryExpression* node) {};
    virtual void onFinishVisit(BinaryExpression* node) {};
    virtual void onFinishVisit(CallableExpression* node) {};
    virtual void onFinishVisit(AccessExpression* node) {};
    virtual void onFinishVisit(BlockStatement* node) {};
    virtual void onFinishVisit(KeywordStatement* node) {};
    virtual void onFinishVisit(ExpressionStatement* node) {};
    virtual void onFinishVisit(AssignmentStatement* node) {};
    virtual void onFinishVisit(ForStatement* node) {};
    virtual void onFinishVisit(WhileStatement* node) {};
    virtual void onFinishVisit(ForRangeStatement* node) {};
    virtual void onFinishVisit(ReturnStatement* node) {};
    virtual void onFinishVisit(IfStatement* node) {};
    virtual void onFinishVisit(SwitchCaseClause* node) {};
    virtual void onFinishVisit(SwitchStatement* node) {};
    virtual void onFinishVisit(DeclarationStatement* node) {};
    virtual void onFinishVisit(IdentifiersWithType* node) {};
    virtual void onFinishVisit(FunctionSignature* node) {};
    virtual void onFinishVisit(ArraySignature* node) {};
    virtual void onFinishVisit(StructSignature* node) {};
    virtual void onFinishVisit(IdentifierAsType* node) {};
    virtual void onFinishVisit(InterfaceType* node) {};
    virtual void onFinishVisit(CompositeLiteral* node) {};
    virtual void onFinishVisit(ElementCompositeLiteral* node) {};
    virtual void onFinishVisit(IntegerExpression* node) {};

    virtual ~Visitor() = default;
};


// thin/empty implementation
// not inheriting std exceptions
// and not inheritable
struct ProcessingInterrupted final {};