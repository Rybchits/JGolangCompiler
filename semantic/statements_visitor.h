#pragma once

#include "../visitor.h"

#include <stack>
#include <string>
#include <vector>

class StatementsVisitor : public Visitor {
private:
    std::vector<std::string> errors;

    void addError(const std::string& message) { errors.push_back(message); }

    bool insideSwitchCaseClause = false;

    std::stack<StatementASTPtr> nextIterationsLoops;
    static const std::string indexPrivateVariableName;

    void transformAssignment(AssignmentStatement* assigmnent);
    StatementList transformStatements(StatementList list);

    StatementASTPtr transformIfStatement(StatementASTPtr stmt);
    StatementASTPtr transformSwitchStatement(StatementASTPtr stmt);
    BlockStatementPtr transformForToWhile(ForStatement* forStmt);
    BlockStatementPtr transformForRangeToWhile(ForRangeStatement *forRangeStmt);

    StatementList transformKeywordStatements(StatementList body);

    bool checkReturnStatements(StatementAST* stmt);
    bool checkReturnStatements(BlockStatement* block);
    bool checkReturnStatements(IfStatement* ifStatement);
    bool checkReturnStatements(SwitchStatement* switchStmt);

public:

    void onStartVisit(ForStatement& node) override;
    void onFinishVisit(ForStatement& node) override;

    void onStartVisit(WhileStatement& node) override;
    void onFinishVisit(WhileStatement& node) override;

    void onStartVisit(ForRangeStatement& node) override;
    void onFinishVisit(ForRangeStatement& node) override;

    void onStartVisit(BlockStatement& node) override;
    void onFinishVisit(BlockStatement& node) override;

    void onFinishVisit(IfStatement& node) override;

    void onFinishVisit(FunctionDeclaration& node) override;

    void onStartVisit(SwitchCaseClause& node) override;
    void onFinishVisit(SwitchCaseClause& node) override;

    void onFinishVisit(SwitchStatement& node) override;

    const std::vector<std::string>& getErrors() const { return errors; }
    void transform(PackageAST* packageAst);
    ~StatementsVisitor() override = default;
};
