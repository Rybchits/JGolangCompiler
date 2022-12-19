#include "../visitor.h"
#include "java_entity.h"
#include "semantic.h"

#include <unordered_map>
#include <stack>

class TreeTransformationVisitor : public Visitor {
private:
    Semantic* semantic;

    std::stack<StatementAST*> nextIterationsLoops;

    size_t indexClassInDeclaration = 0;
    std::string currentFunctionName = "";
    std::string currentAliasTypeDecl = "";

    std::unordered_map<std::string, JavaClass> classes;

    std::string getNameCurrentClass();
    StatementList transformForToWhile(ForStatement* forStmt);
    StatementList transformForRangeToWhile(ForRangeStatement* forRangeStmt);

public:
    void onStartVisit(FunctionDeclaration* node) override;
    void onStartVisit(MethodDeclaration* node) override;
    void onStartVisit(TypeDeclaration* node) override;

    void onFinishVisit(FunctionDeclaration* node) override;
    void onFinishVisit(MethodDeclaration* node) override;
    void onFinishVisit(TypeDeclaration* node) override;
    
    void onFinishVisit(IdentifiersWithType* node) override;
    void onFinishVisit(ArraySignature* node) override;
    void onFinishVisit(CompositeLiteral* node) override;

    void onFinishVisit(BlockStatement* node) override;

    void onStartVisit(ForStatement* node) override;

public:
    std::unordered_map<std::string, JavaClass> transform(PackageAST* packageAst);
    explicit TreeTransformationVisitor(Semantic* semantic): semantic(semantic) {};
    ~TreeTransformationVisitor() override = default;
};