#pragma once

#include "enums.h"

#include <list>
#include <string>
#include <string_view>
#include <variant>
#include <algorithm>

class NodeAST;
class DeclarationAST;
class VariableDeclaration;
class TypeDeclaration;
class StatementAST;
class BlockStatement;
class SwitchCaseClause;
class TypeAST;
class FunctionDeclaration;
class IdentifiersWithType;
class ExpressionAST;
class FunctionSignature;
class ElementCompositeLiteral;

// typedefs
typedef std::list<DeclarationAST *> DeclarationList;
typedef std::list<std::string> IdentifiersList;
typedef std::list<StatementAST *> StatementList;
typedef std::list<SwitchCaseClause *> SwitchCaseList;
typedef std::list<ExpressionAST *> ExpressionList;
typedef std::list<TypeAST *> TypeList;
typedef std::list<ElementCompositeLiteral *> ElementCompositeLiteralList;
typedef std::list<FunctionDeclaration *> FunctionList;

// Functions
std::list<IdentifiersWithType *>* AttachIdentifiersToListTypes(TypeList& listTypes);
TypeList* ListIdentifiersToListTypes(IdentifiersList& identifiers);

template <typename T>
T* As(NodeAST* node);

class NodeAST {
private:
    inline static int64_t LastNodeId = 1;
public:
    const int64_t nodeId;
    virtual ~NodeAST() = default;
    [[maybe_unused]] virtual void toDot(std::ostream& out) const noexcept = 0;

protected:
    NodeAST() : nodeId(LastNodeId++) {}
    [[nodiscard]] virtual std::string name() const noexcept = 0;
};


class PackageAST : public NodeAST {
public:
    explicit PackageAST(const std::string_view package, std::list<DeclarationAST *> *decls) : packageName(package),
                                                                                              topDeclarations(decls) {}

    const std::string packageName;
    const std::list<DeclarationAST *> *topDeclarations;

    [[nodiscard]] std::string name() const noexcept override { return "Package"; }
    void toDot(std::ostream &out) const noexcept override;
};


class DeclarationAST : public NodeAST {
public:
    [[nodiscard]] std::string name() const noexcept override = 0;
    void toDot(std::ostream &out) const noexcept override = 0;
};



/* -------------------------------- Declaration -------------------------------- */
class VariableDeclaration : public DeclarationAST {
public:
    bool isConst;
    IdentifiersWithType *identifiersWithType;
    ExpressionList values;

    VariableDeclaration(IdentifiersWithType *typedIds, ExpressionList &values, bool isConst)
            : identifiersWithType(typedIds), values(values), isConst(isConst) {};

    [[nodiscard]] std::string name() const noexcept override { return "VarDecl"; };
    void toDot(std::ostream &out) const noexcept override;
};


class TypeDeclaration : public DeclarationAST {
public:
    std::string alias;
    TypeAST *declType;

    TypeDeclaration(const std::string_view id, TypeAST *type) : alias(id), declType(type) {};

    [[nodiscard]] std::string name() const noexcept override { return "TypeDecl"; };
    void toDot(std::ostream &out) const noexcept override;
};


class FunctionDeclaration : public DeclarationAST {
public:
    std::string identifier;
    FunctionSignature *signature;
    BlockStatement *block;

    FunctionDeclaration(const std::string_view id, FunctionSignature *signature, BlockStatement *stmt)
            : identifier(id), signature(signature), block(stmt) {};

    [[nodiscard]] std::string name() const noexcept override { return "FuncDecl"; };
    void toDot(std::ostream &out) const noexcept override;
};


class MethodDeclaration : public FunctionDeclaration {
public:
    std::string receiverIdentifier;
    TypeAST *receiverType;

    MethodDeclaration(const std::string_view id, const std::string_view recId, TypeAST *recType,
                      FunctionSignature *signature, BlockStatement *stmt) :
            FunctionDeclaration(id, signature, stmt), receiverType(recType), receiverIdentifier(recId) {};

    [[nodiscard]] std::string name() const noexcept override { return "MethodDecl"; };
    void toDot(std::ostream &out) const noexcept override;
};



/* -------------------------------- Expression -------------------------------- */
class ExpressionAST : public NodeAST {
public:
    [[nodiscard]] std::string name() const noexcept override = 0;
    void toDot(std::ostream &out) const noexcept override = 0;
};


class IdentifierAsExpression : public ExpressionAST {
public:
    std::string identifier;

    explicit IdentifierAsExpression(const std::string_view id) : identifier(id) {};

    [[nodiscard]] std::string name() const noexcept override { return "IdExpr"; };
    void toDot(std::ostream &out) const noexcept override;
};


class IntegerExpression : public ExpressionAST {
public:
    long long intLit;

    explicit IntegerExpression(long long i) : intLit(i) {};

    [[nodiscard]] std::string name() const noexcept override { return "IntegerLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class BooleanExpression : public ExpressionAST {
public:
    bool boolLit;

    explicit BooleanExpression(long long boolean) : boolLit(boolean) {};

    [[nodiscard]] std::string name() const noexcept override { return "BooleanLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class FloatExpression : public ExpressionAST {
public:
    double floatLit;

    explicit FloatExpression(double floating) : floatLit(floating) {};

    [[nodiscard]] std::string name() const noexcept override { return "FloatLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class StringExpression : public ExpressionAST {
public:
    std::string stringLit;

    explicit StringExpression(const std::string_view string) : stringLit(string) {};

    [[nodiscard]] std::string name() const noexcept override { return "StringLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class RuneExpression : public ExpressionAST {
public:
    int32_t runeLit;

    explicit RuneExpression(int32_t rune) : runeLit(rune) {};

    [[nodiscard]] std::string name() const noexcept override { return "RuneLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class NilExpression : public ExpressionAST {
public:
    NilExpression() = default;

    [[nodiscard]] std::string name() const noexcept override { return "NilLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class FunctionLitExpression : public ExpressionAST {
public:
    FunctionSignature *signature;
    BlockStatement *block;

    FunctionLitExpression(FunctionSignature *signature, BlockStatement *block) : signature(signature), block(block) {};

    [[nodiscard]] std::string name() const noexcept override { return "FunctionLit"; };
    void toDot(std::ostream &out) const noexcept override;
};


class UnaryExpression : public ExpressionAST {
public:
    UnaryExpressionEnum type;
    ExpressionAST *expression;

    explicit UnaryExpression(UnaryExpressionEnum type, ExpressionAST *expr) : type(type), expression(expr) {};

    [[nodiscard]] std::string name() const noexcept override;
    void toDot(std::ostream &out) const noexcept override;
};


class BinaryExpression : public ExpressionAST {
public:
    BinaryExpressionEnum type;
    ExpressionAST *lhs;
    ExpressionAST *rhs;

    BinaryExpression(BinaryExpressionEnum type, ExpressionAST *lhs, ExpressionAST *rhs)
            : type(type), lhs(lhs), rhs(rhs) {};

    [[nodiscard]] std::string name() const noexcept override;
    void toDot(std::ostream &out) const noexcept override;
};


// Call function or Conversion
class CallableExpression : public ExpressionAST {
public:
    ExpressionAST *base;
    ExpressionList arguments;

    CallableExpression(ExpressionAST *base, ExpressionList &args) : base(base), arguments(args) {};

    [[nodiscard]] std::string name() const noexcept override { return "CallableExpr"; };
    void toDot(std::ostream &out) const noexcept override;
};


class AccessExpression : public ExpressionAST {
public:
    AccessExpressionEnum type;
    ExpressionAST *base;
    ExpressionAST *accessor;

    AccessExpression(AccessExpressionEnum type, ExpressionAST *base, ExpressionAST *accessor)
            : type(type), base(base), accessor(accessor) {};

    [[nodiscard]] std::string name() const noexcept override;
    void toDot(std::ostream &out) const noexcept override;
};


class ElementCompositeLiteral : public NodeAST {
public:
    ExpressionAST *key;

    std::variant<ExpressionAST *, ElementCompositeLiteralList> value;

    ElementCompositeLiteral(ExpressionAST *key, ExpressionAST *value) : key(key), value(value) {};

    ElementCompositeLiteral(ExpressionAST *key, ElementCompositeLiteralList &value)
            : key(key), value(value) {};

    [[nodiscard]] std::string name() const noexcept override { return "ElementComposite"; };
    void toDot(std::ostream &out) const noexcept override;
};


class CompositeLiteral : public ExpressionAST {
public:
    TypeAST *type;
    ElementCompositeLiteralList elements;

    CompositeLiteral(TypeAST *type, std::list<ElementCompositeLiteral *> &elems) : type(type), elements(elems) {};

    [[nodiscard]] std::string name() const noexcept override { return "CompositeLit"; };
    void toDot(std::ostream &out) const noexcept override;
};



/* -------------------------------- Statement -------------------------------- */
class StatementAST : public NodeAST {
public:
    [[nodiscard]] std::string name() const noexcept override = 0;
    void toDot(std::ostream &out) const noexcept override = 0;
};


class BlockStatement : public StatementAST {
public:
    StatementList body;

    explicit BlockStatement(StatementList& list) : body(list) {};

    [[nodiscard]] std::string name() const noexcept override { return "BlockStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class KeywordStatement : public StatementAST {
public:
    KeywordEnum type;

    explicit KeywordStatement(KeywordEnum type) : type(type) {};

    [[nodiscard]] std::string name() const noexcept override;
    void toDot(std::ostream &out) const noexcept override;
};


class ExpressionStatement : public StatementAST {
public:
    ExpressionAST *expression;

    explicit ExpressionStatement(ExpressionAST *expr) : expression(expr) {};

    [[nodiscard]] std::string name() const noexcept override { return "ExprStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class AssignmentStatement : public StatementAST {
public:
    AssignmentEnum type;
    ExpressionList lhs;
    ExpressionList rhs;

    AssignmentStatement(AssignmentEnum type, ExpressionList& lhs, ExpressionList& rhs) : type(type), lhs(lhs),
                                                                                           rhs(rhs) {};

    [[nodiscard]] std::string name() const noexcept override;
    void toDot(std::ostream &out) const noexcept override;
};


class ForStatement : public StatementAST {
public:
    StatementAST *initStatement;
    ExpressionAST *conditionExpression;
    StatementAST *iterationStatement;
    BlockStatement *block;

    ForStatement(StatementAST *init, ExpressionAST *cond, StatementAST *next, BlockStatement *block)
            : initStatement(init), conditionExpression(cond), iterationStatement(next), block(block) {};

    [[nodiscard]] std::string name() const noexcept override { return "ForStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class WhileStatement : public StatementAST {
public:
    ExpressionAST *conditionExpression;
    BlockStatement *block;

    WhileStatement(ExpressionAST *cond, BlockStatement *block) : conditionExpression(cond), block(block) {};

    [[nodiscard]] std::string name() const noexcept override { return "WhileStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class ForRangeStatement : public StatementAST {
public:
    bool hasShortDeclaration;
    ExpressionList initStatement;
    ExpressionAST *expressionValue;
    BlockStatement *block;

    ForRangeStatement(ExpressionList &init, ExpressionAST *val, BlockStatement *block, bool isShort)
            : initStatement(init), hasShortDeclaration(isShort), expressionValue(val), block(block) {};

    [[nodiscard]] std::string name() const noexcept override { return "ForRangeStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class ReturnStatement : public StatementAST {
public:
    ExpressionList returnValues;

    explicit ReturnStatement(ExpressionList &values) : returnValues(values) {};

    [[nodiscard]] std::string name() const noexcept override { return "ReturnStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class IfStatement : public StatementAST {
public:
    StatementAST *preStatement;
    ExpressionAST *condition;
    BlockStatement *thenStatement;
    StatementAST *elseStatement;

    IfStatement(StatementAST *pre, ExpressionAST *cond, BlockStatement *then, StatementAST *elseStmt)
            : preStatement(pre), condition(cond), thenStatement(then), elseStatement(elseStmt) {};

    [[nodiscard]] std::string name() const noexcept override { return "IfStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class SwitchCaseClause : public StatementAST {
public:
    ExpressionAST *expressionCase;
    StatementList statementsList;

    SwitchCaseClause(ExpressionAST *key, StatementList &stmts) : expressionCase(key), statementsList(stmts) {};

    [[nodiscard]] std::string name() const noexcept override { return "CaseStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class SwitchStatement : public StatementAST {
public:
    StatementAST *statement;
    ExpressionAST *expression;
    std::list<SwitchCaseClause *> clauseList;
    StatementList defaultStatement;

    SwitchStatement(StatementAST *init, ExpressionAST *expr, std::list<SwitchCaseClause *> &cases,
                    StatementList &defaultCase)
            : statement(init), expression(expr), clauseList(cases), defaultStatement(defaultCase) {};

    [[nodiscard]] std::string name() const noexcept override { return "SwitchStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};


class DeclarationStatement : public  StatementAST {
public:
    DeclarationList declarations;
    explicit DeclarationStatement(DeclarationList& decls) : declarations(decls) {};

    [[nodiscard]] std::string name() const noexcept override { return "DeclStmt"; }
    void toDot(std::ostream &out) const noexcept override;
};



/* -------------------------------- Types -------------------------------- */
class TypeAST : public NodeAST {
public:
    bool isVariadic;
    bool isPointer;
    [[nodiscard]] std::string name() const noexcept override = 0;
    void toDot(std::ostream &out) const noexcept override = 0;

protected:
    explicit TypeAST(bool isVariadic = false, bool isPointer = false) : isPointer(isPointer), isVariadic(isVariadic) {}
};


class IdentifiersWithType : public NodeAST {
public:
    const IdentifiersList identifiers;
    const TypeAST* type;

    IdentifiersWithType(IdentifiersList& ids, TypeAST* type) : identifiers(ids), type(type) {};
    [[nodiscard]] std::string name() const noexcept override { return "TypedIds"; };
    void toDot(std::ostream &out) const noexcept override;
};


class FunctionSignature : public TypeAST {
public:
    const std::list<IdentifiersWithType *> idsAndTypesArgs{};
    const std::list<IdentifiersWithType *> idsAndTypesResults{};

    FunctionSignature(std::list<IdentifiersWithType *> &args, std::list<IdentifiersWithType *> &results)
            : idsAndTypesArgs(args), idsAndTypesResults(results) {};

    [[nodiscard]] std::string name() const noexcept override { return "TypeFunction"; };
    void toDot(std::ostream &out) const noexcept override;
};


class ArraySignature : public TypeAST {
public:
    const TypeAST *arrayElementType;
    const int dimensions;

    ArraySignature(TypeAST *type, int dims): arrayElementType(type), dimensions(dims) {};

    explicit ArraySignature(TypeAST *type): arrayElementType(type), dimensions(-1) {};

    [[nodiscard]] std::string name() const noexcept override { return "TypeArray"; };
    void toDot(std::ostream &out) const noexcept override;
};


class StructSignature : public TypeAST {
public:
    const std::list<IdentifiersWithType *> structMembers;

    explicit StructSignature(std::list<IdentifiersWithType *>& members): structMembers(members) {};

    [[nodiscard]] std::string name() const noexcept override { return "TypeStruct"; };
    void toDot(std::ostream &out) const noexcept override;
};


class IdentifierAsType : public TypeAST {
public:
    std::string identifier;

    explicit IdentifierAsType(const std::string_view id): identifier(id) {};

    [[nodiscard]] std::string name() const noexcept override { return "TypeIdentifier"; };
    void toDot(std::ostream &out) const noexcept override;

    [[nodiscard]] bool isBuiltInType() const;
};


class InterfaceType : public TypeAST {
public:
    FunctionList functions;

    InterfaceType(FunctionList& list) : functions(list) {};

    [[nodiscard]] std::string name() const noexcept override { return "InterfaceType"; };
    void toDot(std::ostream &out) const noexcept override;
};
