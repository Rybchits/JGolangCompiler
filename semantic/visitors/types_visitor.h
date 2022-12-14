#include "../semantic.h"
#include "../../context.h"

#include <vector>

class TypesVisitor;

class ConstExpressionVisitor : private Visitor {
private:
    TypesVisitor* typesVisitor;
    bool constValid = true;

    void onFinishVisit(IdentifierAsExpression* node);
    void onFinishVisit(CallableExpression* node);
    void onFinishVisit(AccessExpression* node);
    void onFinishVisit(CompositeLiteral* node);

public:
    bool isConstExpression(ExpressionAST* expr);
    ConstExpressionVisitor(TypesVisitor* visitor): typesVisitor(visitor) {};
};


class TypesVisitor : public Visitor {
friend class ConstExpressionVisitor;
private:
    Semantic* semantic;
    Context<VariableEntity*> scopesDeclarations;
    std::unordered_map<size_t, TypeEntity*> typesExpressions;

    MethodEntity* currentMethodEntity;
    bool lastAddedScopeInFuncDecl = false;
    int numberLocalVariables = 0;

    void onStartVisit(BlockStatement* node);
    void onFinishVisit(BlockStatement* node);
    
    void onStartVisit(ExpressionStatement* node);
    void onFinishVisit(VariableDeclaration* node);
    void onFinishVisit(ShortVarDeclarationStatement* node);
    void onFinishVisit(AssignmentStatement* node);
    void onFinishVisit(ReturnStatement* node);
    void onFinishVisit(WhileStatement* node);
    void onFinishVisit(IfStatement* node);

    void onFinishVisit(IdentifierAsExpression* node);
    void onFinishVisit(IntegerExpression* node);
    void onFinishVisit(BooleanExpression* node);
    void onFinishVisit(FloatExpression* node);
    void onFinishVisit(StringExpression* node);
    void onFinishVisit(RuneExpression* node);
    void onFinishVisit(NilExpression* node);
    void onFinishVisit(UnaryExpression* node);
    void onFinishVisit(BinaryExpression* node);
    void onFinishVisit(CallableExpression* node);
    void onFinishVisit(AccessExpression* node);
    void onFinishVisit(CompositeLiteral* node);
    void onFinishVisit(ElementCompositeLiteral* node);


public:
    explicit TypesVisitor(Semantic* semantic): semantic(semantic) {};
    std::unordered_map<size_t, TypeEntity*> getTypesExpressions() const;
    ClassEntity* createGlobalClass(std::list<FunctionDeclaration*>, std::list<VariableDeclaration*>& globalVariables);
};