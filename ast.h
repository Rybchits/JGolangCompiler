#pragma once

#include "enums.h"
#include "visitor.h"

#include <list>
#include <string>
#include <string_view>
#include <variant>
#include <algorithm>

class Visitor;

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
class IdentifierAsType;
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
IdentifiersList* IdentifiersListFromExpressions(ExpressionList& expressions);

class NodeAST {
private:
    inline static int64_t LastNodeId = 1;
public:
    const int64_t nodeId;
    virtual ~NodeAST() = default;
    virtual void acceptVisitor(Visitor* visitor) noexcept = 0;
    virtual NodeAST* clone() const = 0;
protected:
    NodeAST() : nodeId(LastNodeId++) {}
    [[nodiscard]] virtual std::string name() const noexcept = 0;
};


class PackageAST : public NodeAST {
public:
    explicit PackageAST(const std::string_view package, DeclarationList& decls) : packageName(package),
                                                                                              topDeclarations(decls) {}
    
    PackageAST* clone() const noexcept override;

    const std::string packageName;
    const DeclarationList topDeclarations;

    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "Package"; }
};


class DeclarationAST : public NodeAST {
public:
    DeclarationAST* clone() const noexcept override = 0;
    void acceptVisitor(Visitor* visitor) noexcept override = 0;
    [[nodiscard]] std::string name() const noexcept override = 0;
};


/* -------------------------------- Declaration -------------------------------- */
class VariableDeclaration : public DeclarationAST {
public:
    bool isConst;
    IdentifiersWithType *identifiersWithType;
    ExpressionList values;

    VariableDeclaration(IdentifiersWithType *typedIds, ExpressionList &values, bool isConst = false)
            : identifiersWithType(typedIds), values(values), isConst(isConst) {};
        
    VariableDeclaration* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "VarDecl"; };
};


class TypeDeclaration : public DeclarationAST {
public:
    std::string alias;
    TypeAST *declType;

    TypeDeclaration(const std::string_view id, TypeAST *type) : alias(id), declType(type) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    TypeDeclaration* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "TypeDecl"; };
};


class FunctionDeclaration : public DeclarationAST {
public:
    std::string identifier;
    FunctionSignature *signature;
    BlockStatement *block;

    FunctionDeclaration(const std::string_view id, FunctionSignature *signature, BlockStatement *stmt)
            : identifier(id), signature(signature), block(stmt) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    FunctionDeclaration* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "FuncDecl"; };
};


class MethodDeclaration : public FunctionDeclaration {
public:
    std::string receiverIdentifier;
    IdentifierAsType *receiverType;

    MethodDeclaration(const std::string_view id, const std::string_view recId, IdentifierAsType *recType,
                      FunctionSignature *signature, BlockStatement *stmt) :
            FunctionDeclaration(id, signature, stmt), receiverType(recType), receiverIdentifier(recId) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    MethodDeclaration* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "MethodDecl"; };
};



/* -------------------------------- Expression -------------------------------- */
class ExpressionAST : public NodeAST {
public:
    void acceptVisitor(Visitor* visitor) noexcept override = 0;
    ExpressionAST* clone() const noexcept override = 0;
    [[nodiscard]] std::string name() const noexcept override = 0;
};


class IdentifierAsExpression : public ExpressionAST {
public:
    std::string identifier;

    explicit IdentifierAsExpression(const std::string_view id) : identifier(id) {};
    IdentifierAsExpression* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    
    [[nodiscard]] std::string name() const noexcept override { return "IdExpr"; };
};


class IntegerExpression : public ExpressionAST {
public:
    long long intLit;

    explicit IntegerExpression(long long i) : intLit(i) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    IntegerExpression* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "IntegerLit"; };
};


class BooleanExpression : public ExpressionAST {
public:
    bool boolLit;

    explicit BooleanExpression(long long boolean) : boolLit(boolean) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    BooleanExpression* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "BooleanLit"; };
};


class FloatExpression : public ExpressionAST {
public:
    double floatLit;

    explicit FloatExpression(double floating) : floatLit(floating) {};
    FloatExpression* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "FloatLit"; };
};


class StringExpression : public ExpressionAST {
public:
    std::string stringLit;

    explicit StringExpression(const std::string_view string) : stringLit(string) {};
    StringExpression* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "StringLit"; };
};


class RuneExpression : public ExpressionAST {
public:
    int32_t runeLit;

    explicit RuneExpression(int32_t rune) : runeLit(rune) {};
    RuneExpression* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "RuneLit"; };
};


class NilExpression : public ExpressionAST {
public:
    NilExpression() = default;

    void acceptVisitor(Visitor* visitor) noexcept override;
    NilExpression* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "NilLit"; };
};


class FunctionLitExpression : public ExpressionAST {
public:
    FunctionSignature *signature;
    BlockStatement *block;

    FunctionLitExpression(FunctionSignature *signature, BlockStatement *block) : signature(signature), block(block) {};
    FunctionLitExpression* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "FunctionLit"; };
};


class UnaryExpression : public ExpressionAST {
public:
    UnaryExpressionEnum type;
    ExpressionAST *expression;

    explicit UnaryExpression(UnaryExpressionEnum type, ExpressionAST *expr) : type(type), expression(expr) {};
    UnaryExpression* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
};


class BinaryExpression : public ExpressionAST {
public:
    BinaryExpressionEnum type;
    ExpressionAST *lhs;
    ExpressionAST *rhs;

    BinaryExpression(BinaryExpressionEnum type, ExpressionAST *lhs, ExpressionAST *rhs)
            : type(type), lhs(lhs), rhs(rhs) {};

    BinaryExpression* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
};


// Call function or Conversion
class CallableExpression : public ExpressionAST {
public:
    ExpressionAST *base;
    ExpressionList arguments;

    CallableExpression(ExpressionAST *base, ExpressionList &args) : base(base), arguments(args) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    CallableExpression* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "CallableExpr"; };
};


class AccessExpression : public ExpressionAST {
public:
    AccessExpressionEnum type;
    ExpressionAST *base;
    ExpressionAST *accessor;

    AccessExpression(AccessExpressionEnum type, ExpressionAST *base, ExpressionAST *accessor)
            : type(type), base(base), accessor(accessor) {};

    [[nodiscard]] std::string name() const noexcept override;
    AccessExpression* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
};


class ElementCompositeLiteral : public ExpressionAST {
public:
    ExpressionAST *key;

    std::variant<ExpressionAST *, ElementCompositeLiteralList> value;

    ElementCompositeLiteral(ExpressionAST *key, ExpressionAST *value) : key(key), value(value) {};

    ElementCompositeLiteral(ExpressionAST *key, ElementCompositeLiteralList &value)
            : key(key), value(value) {};

    [[nodiscard]] std::string name() const noexcept override { return "ElementComposite"; };
    void acceptVisitor(Visitor* visitor) noexcept override;
    ElementCompositeLiteral* clone() const noexcept override;
};


class CompositeLiteral : public ExpressionAST {
public:
    TypeAST *type;
    ElementCompositeLiteralList elements;

    CompositeLiteral(TypeAST *type, std::list<ElementCompositeLiteral *> &elems) : type(type), elements(elems) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    CompositeLiteral* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "CompositeLit"; };
};



/* -------------------------------- Statement -------------------------------- */
class StatementAST : public NodeAST {
public:
    void acceptVisitor(Visitor* visitor) noexcept override = 0;
    StatementAST* clone() const noexcept override = 0;
    [[nodiscard]] std::string name() const noexcept override = 0;
};


class BlockStatement : public StatementAST {
public:
    StatementList body;

    explicit BlockStatement(StatementList& list) : body(list) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    BlockStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "BlockStmt"; }
};


class KeywordStatement : public StatementAST {
public:
    KeywordEnum type;

    explicit KeywordStatement(KeywordEnum type) : type(type) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    KeywordStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override;
};


class ExpressionStatement : public StatementAST {
public:
    ExpressionAST *expression;

    explicit ExpressionStatement(ExpressionAST *expr) : expression(expr) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    ExpressionStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "ExprStmt"; }
};


class ShortVarDeclarationStatement : public StatementAST {
public:
    ExpressionList values;
    IdentifiersList identifiers;

    ShortVarDeclarationStatement(IdentifiersList& ids, ExpressionList& values): values(values), identifiers(ids) {};
    
    void acceptVisitor(Visitor* visitor) noexcept override;
    ShortVarDeclarationStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "Op :="; }
};


class AssignmentStatement : public StatementAST {
public:
    AssignmentEnum type;
    ExpressionList lhs;
    ExpressionList rhs;

    ExpressionList indexes;

    AssignmentStatement(AssignmentEnum type, ExpressionAST* lExp, ExpressionAST* rExp);

    AssignmentStatement(AssignmentEnum type, ExpressionList& lhs, ExpressionList& rhs);

    [[nodiscard]] std::string name() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
    AssignmentStatement* clone() const noexcept override;
};


class ForStatement : public StatementAST {
public:
    StatementAST *initStatement;
    ExpressionAST *conditionExpression;
    StatementAST *iterationStatement;
    BlockStatement *block;

    ForStatement(StatementAST *init, ExpressionAST *cond, StatementAST *next, BlockStatement *block)
            : initStatement(init), conditionExpression(cond), iterationStatement(next), block(block) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "ForStmt"; }
    ForStatement* clone() const noexcept override;
};


class WhileStatement : public StatementAST {
public:
    ExpressionAST *conditionExpression;
    BlockStatement *block;

    WhileStatement(ExpressionAST *cond, BlockStatement *block) : conditionExpression(cond), block(block) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    WhileStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "WhileStmt"; }
};


class ForRangeStatement : public StatementAST {
public:
    bool hasShortDeclaration;
    ExpressionList initStatement;
    ExpressionAST *expressionValue;
    BlockStatement *block;

    ForRangeStatement(ExpressionList &init, ExpressionAST *val, BlockStatement *block, bool isShort)
            : initStatement(init), hasShortDeclaration(isShort), expressionValue(val), block(block) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "ForRangeStmt"; }
    ForRangeStatement* clone() const noexcept override;
};


class ReturnStatement : public StatementAST {
public:
    ExpressionList returnValues;

    explicit ReturnStatement(ExpressionList &values) : returnValues(values) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    ReturnStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "ReturnStmt"; }
};


class IfStatement : public StatementAST {
public:
    StatementAST *preStatement;
    ExpressionAST *condition;
    BlockStatement *thenStatement;
    StatementAST *elseStatement;

    IfStatement(StatementAST *pre, ExpressionAST *cond, BlockStatement *then, StatementAST *elseStmt)
            : preStatement(pre), condition(cond), thenStatement(then), elseStatement(elseStmt) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    IfStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "IfStmt"; }
};


class SwitchCaseClause : public StatementAST {
public:
    ExpressionAST *expressionCase;
    StatementList statementsList;

    SwitchCaseClause(ExpressionAST *key, StatementList &stmts) : expressionCase(key), statementsList(stmts) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    SwitchCaseClause* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "CaseStmt"; }
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

    void acceptVisitor(Visitor* visitor) noexcept override;
    SwitchStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "SwitchStmt"; }
};


class DeclarationStatement : public StatementAST {
public:
    DeclarationList declarations;
    explicit DeclarationStatement(DeclarationList& decls) : declarations(decls) {};
    explicit DeclarationStatement(DeclarationAST* decl) : declarations(*(new DeclarationList{ decl })) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    DeclarationStatement* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "DeclStmt"; }
};



/* -------------------------------- Types -------------------------------- */
class TypeAST : public NodeAST {
public:
    bool isVariadic;
    bool isPointer;

    void acceptVisitor(Visitor* visitor) noexcept override = 0;
    TypeAST* clone() const noexcept override = 0;
    [[nodiscard]] std::string name() const noexcept override = 0;

protected:
    explicit TypeAST(bool isVariadic = false, bool isPointer = false) : isPointer(isPointer), isVariadic(isVariadic) {}
};


class IdentifiersWithType : public NodeAST {
public:
    const IdentifiersList identifiers;
    TypeAST* type;

    void acceptVisitor(Visitor* visitor) noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "TypedIds"; };
    IdentifiersWithType* clone() const noexcept override;

    IdentifiersWithType(IdentifiersList& ids, TypeAST* type) : identifiers(ids), type(type) {};
    IdentifiersWithType(std::string id, TypeAST* type): identifiers( *(new IdentifiersList({ id })) ), type(type) {};
};


class FunctionSignature : public TypeAST {
public:
    const std::list<IdentifiersWithType *> idsAndTypesArgs{};
    const std::list<IdentifiersWithType *> idsAndTypesResults{};

    FunctionSignature(std::list<IdentifiersWithType *> &args, std::list<IdentifiersWithType *> &results)
            : idsAndTypesArgs(args), idsAndTypesResults(results) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    FunctionSignature* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "TypeFunction"; };
};

class ArraySignature : public TypeAST {
public:
    TypeAST *arrayElementType;
    const int dimensions;

    ArraySignature(TypeAST *type, int dims): arrayElementType(type), dimensions(dims) {};

    explicit ArraySignature(TypeAST *type): arrayElementType(type), dimensions(-1) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    ArraySignature* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "TypeArray"; };
};


class StructSignature : public TypeAST {
public:
    const std::list<IdentifiersWithType *> structMembers;

    explicit StructSignature(std::list<IdentifiersWithType *>& members): structMembers(members) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    StructSignature* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "TypeStruct"; };
};


class IdentifierAsType : public TypeAST {
public:
    std::string identifier;

    explicit IdentifierAsType(const std::string_view id): identifier(id) {};

    void acceptVisitor(Visitor* visitor) noexcept override;
    IdentifierAsType* clone() const noexcept override;
    [[nodiscard]] std::string name() const noexcept override { return "TypeIdentifier"; };

    [[nodiscard]] bool isBuiltInType() const;
};


class InterfaceType : public TypeAST {
public:
    FunctionList functions;

    explicit InterfaceType(FunctionList& list) : functions(list) {};

    [[nodiscard]] std::string name() const noexcept override { return "InterfaceType"; };
    InterfaceType* clone() const noexcept override;
    void acceptVisitor(Visitor* visitor) noexcept override;
};
