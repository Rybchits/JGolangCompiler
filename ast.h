#pragma once

#include "visitor.h"
#include "entities/type_entity.h"

#include <list>
#include <string>
#include <string_view>
#include <variant>
#include <algorithm>
#include <memory>
#include <optional>
#include <utility>

class TypeEntity;
class Visitor;

class NodeAST;
class PackageAST;
class DeclarationAST;
class VariableDeclaration;
class TypeDeclaration;
class FunctionDeclaration;
class MethodDeclaration;
class ExpressionAST;
class IdentifierAsExpression;
class IntegerExpression;
class BooleanExpression;
class FloatExpression;
class StringExpression;
class NilExpression;
class FunctionLitExpression;
class UnaryExpression;
class BinaryExpression;
class CallableExpression;
class AccessExpression;
class ElementCompositeLiteral;
class CompositeLiteral;
class StatementAST;
class BlockStatement;
class KeywordStatement;
class ExpressionStatement;
class ShortVarDeclarationStatement;
class AssignmentStatement;
class ForStatement;
class WhileStatement;
class ForRangeStatement;
class ReturnStatement;
class IfStatement;
class SwitchCaseClause;
class SwitchStatement;
class DeclarationStatement;
class TypeAST;
class IdentifiersWithType;
class FunctionSignature;
class ArraySignature;
class StructSignature;
class IdentifierAsType;
class InterfaceType;

using NodeASTPtr = std::unique_ptr<NodeAST>;
using PackageASTPtr = std::unique_ptr<PackageAST>;
using DeclarationASTPtr = std::unique_ptr<DeclarationAST>;
using VariableDeclarationPtr = std::unique_ptr<VariableDeclaration>;
using TypeDeclarationPtr = std::unique_ptr<TypeDeclaration>;
using FunctionDeclarationPtr = std::unique_ptr<FunctionDeclaration>;
using MethodDeclarationPtr = std::unique_ptr<MethodDeclaration>;
using ExpressionASTPtr = std::unique_ptr<ExpressionAST>;
using IdentifierAsExpressionPtr = std::unique_ptr<IdentifierAsExpression>;
using IntegerExpressionPtr = std::unique_ptr<IntegerExpression>;
using BooleanExpressionPtr = std::unique_ptr<BooleanExpression>;
using FloatExpressionPtr = std::unique_ptr<FloatExpression>;
using StringExpressionPtr = std::unique_ptr<StringExpression>;
using NilExpressionPtr = std::unique_ptr<NilExpression>;
using FunctionLitExpressionPtr = std::unique_ptr<FunctionLitExpression>;
using UnaryExpressionPtr = std::unique_ptr<UnaryExpression>;
using BinaryExpressionPtr = std::unique_ptr<BinaryExpression>;
using CallableExpressionPtr = std::unique_ptr<CallableExpression>;
using AccessExpressionPtr = std::unique_ptr<AccessExpression>;
using ElementCompositeLiteralPtr = std::unique_ptr<ElementCompositeLiteral>;
using CompositeLiteralPtr = std::unique_ptr<CompositeLiteral>;
using StatementASTPtr = std::unique_ptr<StatementAST>;
using BlockStatementPtr = std::unique_ptr<BlockStatement>;
using KeywordStatementPtr = std::unique_ptr<KeywordStatement>;
using ExpressionStatementPtr = std::unique_ptr<ExpressionStatement>;
using ShortVarDeclarationStatementPtr = std::unique_ptr<ShortVarDeclarationStatement>;
using AssignmentStatementPtr = std::unique_ptr<AssignmentStatement>;
using ForStatementPtr = std::unique_ptr<ForStatement>;
using WhileStatementPtr = std::unique_ptr<WhileStatement>;
using ForRangeStatementPtr = std::unique_ptr<ForRangeStatement>;
using ReturnStatementPtr = std::unique_ptr<ReturnStatement>;
using IfStatementPtr = std::unique_ptr<IfStatement>;
using SwitchCaseClausePtr = std::unique_ptr<SwitchCaseClause>;
using SwitchStatementPtr = std::unique_ptr<SwitchStatement>;
using DeclarationStatementPtr = std::unique_ptr<DeclarationStatement>;
using TypeASTPtr = std::unique_ptr<TypeAST>;
using IdentifiersWithTypePtr = std::unique_ptr<IdentifiersWithType>;
using FunctionSignaturePtr = std::unique_ptr<FunctionSignature>;
using ArraySignaturePtr = std::unique_ptr<ArraySignature>;
using StructSignaturePtr = std::unique_ptr<StructSignature>;
using IdentifierAsTypePtr = std::unique_ptr<IdentifierAsType>;
using InterfaceTypePtr = std::unique_ptr<InterfaceType>;

using DeclarationList = std::list<DeclarationASTPtr>;
using IdentifiersList = std::list<std::string>;
using StatementList = std::list<StatementASTPtr>;
using SwitchCaseList = std::list<SwitchCaseClausePtr>;
using ExpressionList = std::list<ExpressionASTPtr>;
using TypeList = std::list<TypeASTPtr>;
using ElementCompositeLiteralList = std::list<ElementCompositeLiteralPtr>;
using FunctionList = std::list<FunctionDeclarationPtr>;
using IdentifiersWithTypeList = std::list<IdentifiersWithTypePtr>;

template<typename List, typename... Values>
List MakeList(Values&&... values) {
    List result;
    (result.push_back(std::forward<Values>(values)), ...);
    return result;
}

IdentifiersWithTypeList AttachIdentifiersToListTypes(TypeList listTypes);
TypeList ListIdentifiersToListTypes(const IdentifiersList& identifiers);
std::optional<IdentifiersList> IdentifiersListFromExpressions(const ExpressionList& expressions);

class NodeAST {
private:
    inline static int64_t LastNodeId = 1;
public:
    const int64_t nodeId;
    const int64_t line = 0;

    virtual ~NodeAST() = default;
    virtual void acceptVisitor(Visitor* visitor) = 0;
    NodeASTPtr clone() const { return cloneAs<NodeAST>(); }
protected:
    NodeAST() : nodeId(LastNodeId++) {}
    NodeAST(const NodeAST&) = delete;
    NodeAST& operator=(const NodeAST&) = delete;

    virtual NodeASTPtr cloneImpl() const = 0;
    virtual void copyMetadataTo(NodeAST&) const {}

    template<typename T>
    std::unique_ptr<T> cloneAs() const {
        auto copy = cloneImpl();
        copyMetadataTo(*copy);
        return std::unique_ptr<T>(static_cast<T*>(copy.release()));
    }
public:
    [[nodiscard]] virtual std::string name() const noexcept = 0;
};


class PackageAST : public NodeAST {
public:
    explicit PackageAST(const std::string_view package, DeclarationList decls) : packageName(package),
                                                                                              topDeclarations(std::move(decls)) {}
    
    PackageASTPtr clone() const { return cloneAs<PackageAST>(); }

    const std::string packageName;
    DeclarationList topDeclarations;

    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "Package"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class DeclarationAST : public NodeAST {
public:
    DeclarationASTPtr clone() const { return cloneAs<DeclarationAST>(); }
    void acceptVisitor(Visitor* visitor) override = 0;
    [[nodiscard]] std::string name() const noexcept override = 0;
};


/* -------------------------------- Declaration -------------------------------- */
class VariableDeclaration : public DeclarationAST {
public:
    bool isConst;
    IdentifiersWithTypePtr identifiersWithType;
    ExpressionList values;

    VariableDeclaration(IdentifiersWithTypePtr typedIds, ExpressionList values, bool isConst = false)
            : identifiersWithType(std::move(typedIds)), values(std::move(values)), isConst(isConst) {};
        
    VariableDeclarationPtr clone() const { return cloneAs<VariableDeclaration>(); }
    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "VarDecl"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class TypeDeclaration : public DeclarationAST {
public:
    std::string alias;
    TypeASTPtr declType;

    TypeDeclaration(const std::string_view id, TypeASTPtr type) : alias(id), declType(std::move(type)) {};

    void acceptVisitor(Visitor* visitor) override;
    TypeDeclarationPtr clone() const { return cloneAs<TypeDeclaration>(); }
    [[nodiscard]] std::string name() const noexcept override { return "TypeDecl"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class FunctionDeclaration : public DeclarationAST {
public:
    std::string identifier;
    FunctionSignaturePtr signature;
    BlockStatementPtr block;

    FunctionDeclaration(const std::string_view id, FunctionSignaturePtr signature, BlockStatementPtr stmt)
            : identifier(id), signature(std::move(signature)), block(std::move(stmt)) {};

    void acceptVisitor(Visitor* visitor) override;
    FunctionDeclarationPtr clone() const { return cloneAs<FunctionDeclaration>(); }
    [[nodiscard]] std::string name() const noexcept override { return "FuncDecl"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class MethodDeclaration : public FunctionDeclaration {
public:
    std::string receiverIdentifier;
    IdentifierAsTypePtr receiverType;

    MethodDeclaration(const std::string_view id, const std::string_view recId, IdentifierAsTypePtr recType,
                      FunctionSignaturePtr signature, BlockStatementPtr stmt) :
            FunctionDeclaration(id, std::move(signature), std::move(stmt)), receiverType(std::move(recType)), receiverIdentifier(recId) {};

    void acceptVisitor(Visitor* visitor) override;
    MethodDeclarationPtr clone() const { return cloneAs<MethodDeclaration>(); }
    [[nodiscard]] std::string name() const noexcept override { return "MethodDecl"; };

private:
    NodeASTPtr cloneImpl() const override;
};



/* -------------------------------- Expression -------------------------------- */
class ExpressionAST : public NodeAST {
public:
    TypePtr typeExpression;

protected:
    void copyMetadataTo(NodeAST& copy) const override {
        static_cast<ExpressionAST&>(copy).typeExpression = typeExpression;
    }

public:

    void acceptVisitor(Visitor* visitor) override = 0;
    ExpressionASTPtr clone() const { return cloneAs<ExpressionAST>(); }
    [[nodiscard]] std::string name() const noexcept override = 0;
};


class IdentifierAsExpression : public ExpressionAST {
public:
    std::string identifier;
    bool isDestination = false;

    explicit IdentifierAsExpression(const std::string_view id) : identifier(id) {};
    IdentifierAsExpressionPtr clone() const { return cloneAs<IdentifierAsExpression>(); }
    void acceptVisitor(Visitor* visitor) override;
    
    [[nodiscard]] std::string name() const noexcept override { return "IdExpr"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class IntegerExpression : public ExpressionAST {
public:
    long long intLit;

    explicit IntegerExpression(long long i) : intLit(i) {};

    void acceptVisitor(Visitor* visitor) override;
    IntegerExpressionPtr clone() const { return cloneAs<IntegerExpression>(); }
    [[nodiscard]] std::string name() const noexcept override { return "IntegerLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class BooleanExpression : public ExpressionAST {
public:
    bool boolLit;

    explicit BooleanExpression(long long boolean) : boolLit(boolean) {};

    void acceptVisitor(Visitor* visitor) override;
    BooleanExpressionPtr clone() const { return cloneAs<BooleanExpression>(); }
    [[nodiscard]] std::string name() const noexcept override { return "BooleanLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class FloatExpression : public ExpressionAST {
public:
    double floatLit;

    explicit FloatExpression(double floating) : floatLit(floating) {};
    FloatExpressionPtr clone() const { return cloneAs<FloatExpression>(); }
    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "FloatLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class StringExpression : public ExpressionAST {
public:
    std::string stringLit;

    explicit StringExpression(const std::string_view string) : stringLit(string) {};
    StringExpressionPtr clone() const { return cloneAs<StringExpression>(); }
    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "StringLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class NilExpression : public ExpressionAST {
public:
    NilExpression() = default;

    void acceptVisitor(Visitor* visitor) override;
    NilExpressionPtr clone() const { return cloneAs<NilExpression>(); }
    [[nodiscard]] std::string name() const noexcept override { return "NilLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class FunctionLitExpression : public ExpressionAST {
public:
    FunctionSignaturePtr signature;
    BlockStatementPtr block;

    FunctionLitExpression(FunctionSignaturePtr signature, BlockStatementPtr block) : signature(std::move(signature)), block(std::move(block)) {};
    FunctionLitExpressionPtr clone() const { return cloneAs<FunctionLitExpression>(); }
    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "FunctionLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class UnaryExpression : public ExpressionAST {
public:
    enum UnaryExpressionEnum {
        UnaryNot,
        UnaryPlus,
        UnaryMinus,
        Increment,
        Decrement,
        Variadic,
    } type;

    ExpressionASTPtr expression;

    explicit UnaryExpression(UnaryExpressionEnum type, ExpressionASTPtr expr) : type(type), expression(std::move(expr)) {};
    UnaryExpressionPtr clone() const { return cloneAs<UnaryExpression>(); }
    [[nodiscard]] std::string name() const noexcept override;
    void acceptVisitor(Visitor* visitor) override;

private:
    NodeASTPtr cloneImpl() const override;
};


class BinaryExpression : public ExpressionAST {
public:
    enum BinaryExpressionEnum {
        Addition,            // +
        Subtraction,         // -
        Multiplication,      // *
        Division,            // /
        Mod,                 // %
        And,                 // &&
        Or,                  // ||
        Equal,               // ==
        Greater,             // >
        Less,                // <
        NotEqual,            // !=
        LessOrEqual,         // <=
        GreatOrEqual,        // >=
    } type;

    ExpressionASTPtr lhs;
    ExpressionASTPtr rhs;

    bool isLogical();
    bool isComparison();

    BinaryExpression(BinaryExpressionEnum type, ExpressionASTPtr lhs, ExpressionASTPtr rhs)
            : type(type), lhs(std::move(lhs)), rhs(std::move(rhs)) {};

    BinaryExpressionPtr clone() const { return cloneAs<BinaryExpression>(); }
    [[nodiscard]] std::string name() const noexcept override;
    void acceptVisitor(Visitor* visitor) override;

private:
    NodeASTPtr cloneImpl() const override;
};


// Call function or Conversion
class CallableExpression : public ExpressionAST {
public:
    ExpressionASTPtr base;
    ExpressionList arguments;

    CallableExpression(ExpressionASTPtr base, ExpressionList args) : base(std::move(base)), arguments(std::move(args)) {};

    void acceptVisitor(Visitor* visitor) override;
    CallableExpressionPtr clone() const { return cloneAs<CallableExpression>(); }
    [[nodiscard]] std::string name() const noexcept override { return "CallableExpr"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class AccessExpression : public ExpressionAST {
public:
    ExpressionASTPtr base;
    ExpressionASTPtr accessor;

    enum AccessExpressionEnum {
        Indexing,
        FieldSelect,
    } type;

    AccessExpression(AccessExpressionEnum type, ExpressionASTPtr base, ExpressionASTPtr accessor)
            : type(type), base(std::move(base)), accessor(std::move(accessor)) {};

    [[nodiscard]] std::string name() const noexcept override;
    AccessExpressionPtr clone() const { return cloneAs<AccessExpression>(); }
    void acceptVisitor(Visitor* visitor) override;

private:
    NodeASTPtr cloneImpl() const override;
};


class ElementCompositeLiteral : public ExpressionAST {
public:
    ExpressionASTPtr key;

    std::variant<ExpressionASTPtr, ElementCompositeLiteralList> value;

    ElementCompositeLiteral(ExpressionASTPtr key, ExpressionASTPtr value) : key(std::move(key)), value(std::move(value)) {};

    ElementCompositeLiteral(ExpressionASTPtr key, ElementCompositeLiteralList value)
            : key(std::move(key)), value(std::move(value)) {};

    [[nodiscard]] std::string name() const noexcept override { return "ElementComposite"; };
    void acceptVisitor(Visitor* visitor) override;
    ElementCompositeLiteralPtr clone() const { return cloneAs<ElementCompositeLiteral>(); }

private:
    NodeASTPtr cloneImpl() const override;
};


class CompositeLiteral : public ExpressionAST {
public:
    TypeASTPtr type;
    ElementCompositeLiteralList elements;

    CompositeLiteral(TypeASTPtr type, ElementCompositeLiteralList elems) : type(std::move(type)), elements(std::move(elems)) {};

    void acceptVisitor(Visitor* visitor) override;
    CompositeLiteralPtr clone() const { return cloneAs<CompositeLiteral>(); }
    [[nodiscard]] std::string name() const noexcept override { return "CompositeLit"; };

private:
    NodeASTPtr cloneImpl() const override;
};



/* -------------------------------- Statement -------------------------------- */
class StatementAST : public NodeAST {
public:
    void acceptVisitor(Visitor* visitor) override = 0;
    StatementASTPtr clone() const { return cloneAs<StatementAST>(); }
    [[nodiscard]] std::string name() const noexcept override = 0;
};


class BlockStatement : public StatementAST {
public:
    StatementList body;

    explicit BlockStatement() : body() {};
    explicit BlockStatement(StatementASTPtr  stmt) : body(MakeList<StatementList>(std::move(stmt))) {};
    explicit BlockStatement(StatementList list) : body(std::move(list)) {};

    void acceptVisitor(Visitor* visitor) override;
    BlockStatementPtr clone() const { return cloneAs<BlockStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "BlockStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class KeywordStatement : public StatementAST {
public:
    enum KeywordEnum {
        Break,
        Continue,
        Fallthrough
    } type;

    explicit KeywordStatement(KeywordEnum type) : type(type) {};

    void acceptVisitor(Visitor* visitor) override;
    KeywordStatementPtr clone() const { return cloneAs<KeywordStatement>(); }
    [[nodiscard]] std::string name() const noexcept override;

private:
    NodeASTPtr cloneImpl() const override;
};


class ExpressionStatement : public StatementAST {
public:
    ExpressionASTPtr expression;

    explicit ExpressionStatement(ExpressionASTPtr expr) : expression(std::move(expr)) {};

    void acceptVisitor(Visitor* visitor) override;
    ExpressionStatementPtr clone() const { return cloneAs<ExpressionStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "ExprStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class ShortVarDeclarationStatement : public StatementAST {
public:
    ExpressionList values;
    IdentifiersList identifiers;

    ShortVarDeclarationStatement(IdentifiersList ids, ExpressionList values): values(std::move(values)), identifiers(std::move(ids)) {};
    
    void acceptVisitor(Visitor* visitor) override;
    ShortVarDeclarationStatementPtr clone() const { return cloneAs<ShortVarDeclarationStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "Op :="; }

private:
    NodeASTPtr cloneImpl() const override;
};


class AssignmentStatement : public StatementAST {
public:
    ExpressionList lhs;
    ExpressionList rhs;

    enum AssignmentEnum {
        SimpleAssign,       // =
        MinusAssign,        // -=
        PlusAssign,         // +=
        ModAssign,          // %=
        MulAssign,          // *=
        DivAssign,          // /=
    } type;

    ExpressionList indexes;

    AssignmentStatement(AssignmentEnum type, ExpressionASTPtr lExp, ExpressionASTPtr rExp);

    AssignmentStatement(AssignmentEnum type, ExpressionList lhs, ExpressionList rhs);

    [[nodiscard]] std::string name() const noexcept override;
    void acceptVisitor(Visitor* visitor) override;
    AssignmentStatementPtr clone() const { return cloneAs<AssignmentStatement>(); }

private:
    NodeASTPtr cloneImpl() const override;
};


class ForStatement : public StatementAST {
public:
    StatementASTPtr initStatement;
    ExpressionASTPtr conditionExpression;
    StatementASTPtr iterationStatement;
    BlockStatementPtr block;

    ForStatement(StatementASTPtr init, ExpressionASTPtr cond, StatementASTPtr next, BlockStatementPtr block)
            : initStatement(std::move(init)), conditionExpression(std::move(cond)), iterationStatement(std::move(next)), block(std::move(block)) {};

    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "ForStmt"; }
    ForStatementPtr clone() const { return cloneAs<ForStatement>(); }

private:
    NodeASTPtr cloneImpl() const override;
};


class WhileStatement : public StatementAST {
public:
    ExpressionASTPtr conditionExpression;
    BlockStatementPtr block;

    WhileStatement(ExpressionASTPtr cond, BlockStatementPtr block) : conditionExpression(std::move(cond)), block(std::move(block)) {};

    void acceptVisitor(Visitor* visitor) override;
    WhileStatementPtr clone() const { return cloneAs<WhileStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "WhileStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class ForRangeStatement : public StatementAST {
public:
    bool hasShortDeclaration;
    ExpressionList initStatement;
    ExpressionASTPtr expressionValue;
    BlockStatementPtr block;

    ForRangeStatement(ExpressionList init, ExpressionASTPtr val, BlockStatementPtr block, bool isShort)
            : initStatement(std::move(init)), hasShortDeclaration(isShort), expressionValue(std::move(val)), block(std::move(block)) {};

    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "ForRangeStmt"; }
    ForRangeStatementPtr clone() const { return cloneAs<ForRangeStatement>(); }

private:
    NodeASTPtr cloneImpl() const override;
};


class ReturnStatement : public StatementAST {
public:
    ExpressionList returnValues;

    explicit ReturnStatement(ExpressionList values) : returnValues(std::move(values)) {};

    void acceptVisitor(Visitor* visitor) override;
    ReturnStatementPtr clone() const { return cloneAs<ReturnStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "ReturnStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class IfStatement : public StatementAST {
public:
    StatementASTPtr preStatement;
    ExpressionASTPtr condition;
    BlockStatementPtr thenStatement;
    StatementASTPtr elseStatement;

    IfStatement(StatementASTPtr pre, ExpressionASTPtr cond, BlockStatementPtr then, StatementASTPtr elseStmt)
            : preStatement(std::move(pre)), condition(std::move(cond)), thenStatement(std::move(then)), elseStatement(std::move(elseStmt)) {};

    void acceptVisitor(Visitor* visitor) override;
    IfStatementPtr clone() const { return cloneAs<IfStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "IfStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class SwitchCaseClause : public StatementAST {
public:
    ExpressionASTPtr expressionCase;
    BlockStatementPtr block;
    bool fallthrowEnds = false;

    SwitchCaseClause(ExpressionASTPtr key, BlockStatementPtr  stmts) : expressionCase(std::move(key)), block(std::move(stmts)) {};

    void acceptVisitor(Visitor* visitor) override;
    SwitchCaseClausePtr clone() const { return cloneAs<SwitchCaseClause>(); }
    [[nodiscard]] std::string name() const noexcept override { return "CaseStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class SwitchStatement : public StatementAST {
public:
    StatementASTPtr statement;
    ExpressionASTPtr expression;
    SwitchCaseList clauseList;

    SwitchStatement(StatementASTPtr init, ExpressionASTPtr expr, SwitchCaseList cases)
            : statement(std::move(init)), expression(expr ? std::move(expr) : std::make_unique<BooleanExpression>(true)), clauseList(std::move(cases)) {};

    void acceptVisitor(Visitor* visitor) override;
    SwitchStatementPtr clone() const { return cloneAs<SwitchStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "SwitchStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};


class DeclarationStatement : public StatementAST {
public:
    DeclarationList declarations;
    explicit DeclarationStatement(DeclarationList decls) : declarations(std::move(decls)) {};
    explicit DeclarationStatement(DeclarationASTPtr  decl) : declarations(MakeList<DeclarationList>(std::move(decl))) {};

    void acceptVisitor(Visitor* visitor) override;
    DeclarationStatementPtr clone() const { return cloneAs<DeclarationStatement>(); }
    [[nodiscard]] std::string name() const noexcept override { return "DeclStmt"; }

private:
    NodeASTPtr cloneImpl() const override;
};



/* -------------------------------- Types -------------------------------- */
class TypeAST : public NodeAST {
public:
    bool isVariadic;
    bool isPointer;

    void acceptVisitor(Visitor* visitor) override = 0;
    TypeASTPtr clone() const { return cloneAs<TypeAST>(); }
    [[nodiscard]] std::string name() const noexcept override = 0;

protected:
    void copyMetadataTo(NodeAST& copy) const override {
        auto& typeCopy = static_cast<TypeAST&>(copy);
        typeCopy.isVariadic = isVariadic;
        typeCopy.isPointer = isPointer;
    }

    explicit TypeAST(bool isVariadic = false, bool isPointer = false) : isPointer(isPointer), isVariadic(isVariadic) {}
};


class IdentifiersWithType : public NodeAST {
public:
    IdentifiersList identifiers;
    TypeASTPtr type;

    void acceptVisitor(Visitor* visitor) override;
    [[nodiscard]] std::string name() const noexcept override { return "TypedIds"; };
    IdentifiersWithTypePtr clone() const { return cloneAs<IdentifiersWithType>(); }

    IdentifiersWithType(IdentifiersList ids, TypeASTPtr type) : identifiers(std::move(ids)), type(std::move(type)) {};
    IdentifiersWithType(std::string id, TypeASTPtr type): identifiers({std::move(id)}), type(std::move(type)) {};

private:
    NodeASTPtr cloneImpl() const override;
};


class FunctionSignature : public TypeAST {
public:
    IdentifiersWithTypeList idsAndTypesArgs{};
    IdentifiersWithTypeList idsAndTypesResults{};

    FunctionSignature(IdentifiersWithTypeList args, IdentifiersWithTypeList results)
            : idsAndTypesArgs(std::move(args)), idsAndTypesResults(std::move(results)) {};

    void acceptVisitor(Visitor* visitor) override;
    FunctionSignaturePtr clone() const { return cloneAs<FunctionSignature>(); }
    [[nodiscard]] std::string name() const noexcept override { return "TypeFunction"; };

private:
    NodeASTPtr cloneImpl() const override;
};

class ArraySignature : public TypeAST {
public:
    TypeASTPtr arrayElementType;
    const int dimensions;

    ArraySignature(TypeASTPtr type, int dims): arrayElementType(std::move(type)), dimensions(dims) {};

    explicit ArraySignature(TypeASTPtr type): arrayElementType(std::move(type)), dimensions(-1) {};

    void acceptVisitor(Visitor* visitor) override;
    ArraySignaturePtr clone() const { return cloneAs<ArraySignature>(); }
    [[nodiscard]] std::string name() const noexcept override { return "TypeArray"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class StructSignature : public TypeAST {
public:
    IdentifiersWithTypeList structMembers;

    explicit StructSignature(IdentifiersWithTypeList members): structMembers(std::move(members)) {};

    void acceptVisitor(Visitor* visitor) override;
    StructSignaturePtr clone() const { return cloneAs<StructSignature>(); }
    [[nodiscard]] std::string name() const noexcept override { return "TypeStruct"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class IdentifierAsType : public TypeAST {
public:
    std::string identifier;

    explicit IdentifierAsType(const std::string_view id): identifier(id) {};

    void acceptVisitor(Visitor* visitor) override;
    IdentifierAsTypePtr clone() const { return cloneAs<IdentifierAsType>(); }
    [[nodiscard]] std::string name() const noexcept override { return "TypeIdentifier"; };

private:
    NodeASTPtr cloneImpl() const override;
};


class InterfaceType : public TypeAST {
public:
    FunctionList functions;

    explicit InterfaceType(FunctionList list) : functions(std::move(list)) {};

    [[nodiscard]] std::string name() const noexcept override { return "InterfaceType"; };
    InterfaceTypePtr clone() const { return cloneAs<InterfaceType>(); }
    void acceptVisitor(Visitor* visitor) override;

private:
    NodeASTPtr cloneImpl() const override;
};
