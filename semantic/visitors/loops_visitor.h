#include "../semantic.h"

#include <stack>

class LoopsVisitor : public Visitor {
private:
    Semantic* semantic;

    std::stack<StatementAST*> nextIterationsLoops;
    static const std::string indexPrivateVariableName;

    BlockStatement* transformForToWhile(ForStatement* forStmt);
    BlockStatement* transformForRangeToWhile(ForRangeStatement *forRangeStmt);
    StatementList transformStatementsWithContinues(StatementList body);

public:

    void onStartVisit(ForStatement* node) override;
    void onFinishVisit(ForStatement* node) override;

    void onStartVisit(WhileStatement* node) override;
    void onFinishVisit(WhileStatement* node) override;

    void onStartVisit(ForRangeStatement* node) override;
    void onFinishVisit(ForRangeStatement* node) override;

    void onStartVisit(BlockStatement* node) override;
    void onFinishVisit(BlockStatement* node) override;

    void onStartVisit(SwitchCaseClause* node) override;
    void onStartVisit(SwitchStatement* node) override;

    public:
    void transform(PackageAST* packageAst);
    explicit LoopsVisitor(Semantic* semantic): semantic(semantic) {};
    ~LoopsVisitor() override = default;
};