#include "../semantic.h"
#include <vector>

class TypeCheckVisitor : public Visitor {
private:
    Semantic* semantic;
    std::vector<std::unordered_map<std::string, JavaType>> scopes;
    std::unordered_map<size_t, JavaType> typesNode;

    bool lastAddedScopeInFuncDecl = false;

    void onStartVisit(BlockStatement* node);
    void onFinishVisit(BlockStatement* node);
    
    void onStartVisit(FunctionDeclaration* node);
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

public:
    explicit TypeCheckVisitor(Semantic* semantic): semantic(semantic) {};
    bool check(NodeAST* root);
};