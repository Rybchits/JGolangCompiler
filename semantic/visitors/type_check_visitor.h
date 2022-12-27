#include "../semantic.h"
#include <vector>

class TypeCheckVisitor : public Visitor {
private:
    Semantic* semantic;
    std::vector<std::unordered_map<std::string, VariableEntity*>> scopesDeclarations;
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
    explicit TypeCheckVisitor(Semantic* semantic): semantic(semantic) {};
    ClassEntity* createGlobalClass(std::list<FunctionDeclaration*>, std::list<VariableDeclaration*>& globalVariables);
};