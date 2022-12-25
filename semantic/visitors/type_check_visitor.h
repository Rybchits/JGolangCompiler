#include "../semantic.h"
#include <vector>

class TypeCheckVisitor : public Visitor {
private:
    Semantic* semantic;
    std::vector<std::unordered_map<std::string, JavaType*>> scopesDeclarations;
    std::unordered_map<size_t, JavaType*> typesExpressions;

    bool lastAddedScopeInFuncDecl = false;
    int numberLocalVariables = 0;

    void onStartVisit(BlockStatement* node);
    void onFinishVisit(BlockStatement* node);
    
    void onFinishVisit(VariableDeclaration* node);
    void onFinishVisit(ShortVarDeclarationStatement* node);

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
    bool checkGlobalClass(JavaClass* globalClass, std::list<VariableDeclaration*>& globalVariables);
};