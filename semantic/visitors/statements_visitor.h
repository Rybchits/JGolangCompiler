#include "../semantic.h"

#include <stack>

class StatementsVisitor : public Visitor {
private:
    Semantic* semantic;

    std::stack<StatementAST*> nextIterationsLoops;
    static const std::string indexPrivateVariableName;

    AssignmentStatement* transformAssignment(AssignmentStatement* assigmnent);
    StatementList transformStatements(StatementList& list);

    StatementAST* transformIfStatement(IfStatement* ifStmt);
    StatementAST* transformSwitchStatement(SwitchStatement* switchStmt);
    BlockStatement* transformForToWhile(ForStatement* forStmt);
    BlockStatement* transformForRangeToWhile(ForRangeStatement *forRangeStmt);

    StatementList transformStatementsWithContinues(StatementList body);

    bool checkReturnStatements(StatementAST* stmt);
    bool checkReturnStatements(BlockStatement* block);
    bool checkReturnStatements(IfStatement* ifStatement);
    bool checkReturnStatements(SwitchStatement* switchStmt);

public:

    void onStartVisit(ForStatement* node) override;
    void onFinishVisit(ForStatement* node) override;

    void onStartVisit(WhileStatement* node) override;
    void onFinishVisit(WhileStatement* node) override;

    void onStartVisit(ForRangeStatement* node) override;
    void onFinishVisit(ForRangeStatement* node) override;

    void onStartVisit(BlockStatement* node) override;
    void onFinishVisit(BlockStatement* node) override;

    void onFinishVisit(IfStatement* node) override;
    void onFinishVisit(FunctionDeclaration* node) override;


    void transform(PackageAST* packageAst);
    explicit StatementsVisitor(Semantic* semantic): semantic(semantic) {};
    ~StatementsVisitor() override = default;
};