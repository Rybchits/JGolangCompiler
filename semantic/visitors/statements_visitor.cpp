#include "statements_visitor.h"

const std::string StatementsVisitor::indexPrivateVariableName = "$index";

BlockStatement* StatementsVisitor::transformForToWhile(ForStatement *forStmt) {
    StatementList list;

    if (forStmt->initStatement != nullptr)
        list.push_back(forStmt->initStatement);

    auto whileLoop = new WhileStatement(
            forStmt->conditionExpression == nullptr ? new BooleanExpression(true) : forStmt->conditionExpression,
            forStmt->block
    );

    if (forStmt->iterationStatement != nullptr)
        whileLoop->block->body.push_back(forStmt->iterationStatement);

    list.push_back(whileLoop);

    return new BlockStatement(list);
}

StatementAST* StatementsVisitor::transformIfStatement(IfStatement* ifStmt) {
    if (ifStmt->preStatement) {
        StatementList newStatement;

        newStatement.push_back(ifStmt->preStatement);
        ifStmt->preStatement = nullptr;
        newStatement.push_back(ifStmt);

        return new BlockStatement(newStatement);

    } else {
        return ifStmt;
    }
}

StatementAST* StatementsVisitor::transformSwitchStatement(SwitchStatement *switchStmt) {
    if (switchStmt->statement) {
        StatementList newStatement;

        newStatement.push_back(switchStmt->statement);
        switchStmt->statement = nullptr;
        newStatement.push_back(switchStmt);
        
        return new BlockStatement(newStatement);

    } else {
        return switchStmt;
    }
}


// Перед каждым Continue добавить statement перехода
StatementList StatementsVisitor::transformStatementsWithContinues(StatementList body) {
    StatementList newBody;

    for (auto stmt: body) {
        auto keyword = dynamic_cast<KeywordStatement *>(stmt);

        if (keyword && keyword->type == KeywordEnum::Continue) {

            if (nextIterationsLoops.empty()) {
                semantic->errors.push_back("Continue keyword out of loop");
            } else if (nextIterationsLoops.top() != nullptr) {
                newBody.push_back(nextIterationsLoops.top()->clone());
            }
        }

        newBody.push_back(stmt);
    }
    return newBody;
}

BlockStatement* StatementsVisitor::transformForRangeToWhile(ForRangeStatement *forRangeStmt) {
    StatementList list;

    auto indexDeclaration = new VariableDeclaration(
            new IdentifiersWithType(
                    *(new IdentifiersList{indexPrivateVariableName}), new IdentifierAsType("int")),
            *(new ExpressionList{new IntegerExpression(0)})
    );

    list.push_back(new DeclarationStatement(*(new DeclarationList{indexDeclaration})));

    auto condition = new BinaryExpression(
            BinaryExpressionEnum::Less,
            new IdentifierAsExpression(indexPrivateVariableName),
            new CallableExpression(new IdentifierAsExpression("lenArray"),
                                   *(new ExpressionList{forRangeStmt->expressionValue->clone()}))
    );

    if (forRangeStmt->initStatement.size() > 2) {
        semantic->errors.push_back("Many variables in initialization ForRange loop");
    }

    auto variableForRange = forRangeStmt->initStatement.begin();

    // Index variable
    if (forRangeStmt->initStatement.size() >= 1) {
        StatementAST *indexVariableStatement;

        if (forRangeStmt->hasShortDeclaration) {
            if (auto indexVariableIdentifier = dynamic_cast<IdentifierAsExpression *>(*variableForRange)) {
                indexVariableStatement = new DeclarationStatement(
                        new VariableDeclaration(
                                new IdentifiersWithType(indexVariableIdentifier->identifier, nullptr),
                                *(new ExpressionList({new IdentifierAsExpression(indexPrivateVariableName)}))
                        )
                );
            } else {
                semantic->errors.push_back("Undefined expression (1) in initialization ForRange loop");
            }
        } else {
            indexVariableStatement = new AssignmentStatement(
                    AssignmentEnum::SimpleAssign,
                    *variableForRange,
                    new IdentifierAsExpression(indexPrivateVariableName));
        }

        forRangeStmt->block->body.insert(forRangeStmt->block->body.begin(), indexVariableStatement);
    }

    // Element variable
    if (forRangeStmt->initStatement.size() >= 2) {
        StatementAST *elementVariableStatement;
        variableForRange++;

        ExpressionAST *accessToElement = new AccessExpression(
                AccessExpressionEnum::Indexing,
                forRangeStmt->expressionValue,
                new IdentifierAsExpression(indexPrivateVariableName)
        );

        if (forRangeStmt->hasShortDeclaration) {
            if (auto elementVariableIdentifier = dynamic_cast<IdentifierAsExpression *>(*variableForRange)) {
                elementVariableStatement = new DeclarationStatement(
                        new VariableDeclaration(
                                new IdentifiersWithType(elementVariableIdentifier->identifier, nullptr),
                                *(new ExpressionList({accessToElement}))
                        )
                );
            } else {
                semantic->errors.push_back("Undefined expression (2) in initialization ForRange loop");
            }
        } else {
            elementVariableStatement = new AssignmentStatement(
                    AssignmentEnum::SimpleAssign,
                    *variableForRange,
                    accessToElement);
        }

        forRangeStmt->block->body.insert(forRangeStmt->block->body.begin(), elementVariableStatement);
    }

    forRangeStmt->block->body.insert(forRangeStmt->block->body.end(), new ExpressionStatement(
            new UnaryExpression(
                    UnaryExpressionEnum::Increment,
                    new IdentifierAsExpression(indexPrivateVariableName))));

    list.push_back(new WhileStatement(condition, forRangeStmt->block));
    return new BlockStatement(list);
}

StatementList StatementsVisitor::transformStatements(StatementList& list) {
    StatementList newBody;

    for (auto stmt : list) {

        if (auto forLoop = dynamic_cast<ForStatement *>(stmt)) {
            BlockStatement* transformedLoop = transformForToWhile(forLoop);
            newBody.push_back(transformedLoop);

        } else if (auto forRangeLoop = dynamic_cast<ForRangeStatement *>(stmt)) {
            BlockStatement* transformedLoop = transformForRangeToWhile(forRangeLoop);
            newBody.push_back(transformedLoop);

        } else if (auto ifStatement = dynamic_cast<IfStatement*>(stmt)) {
            newBody.push_back(transformIfStatement(ifStatement));

        } else if (auto switchStatement = dynamic_cast<SwitchStatement*>(stmt)) {
            newBody.push_back(transformSwitchStatement(switchStatement));

        } else {
            newBody.push_back(stmt);
        }
    }
    return newBody;
}

void StatementsVisitor::onFinishVisit(BlockStatement *node) {
    node->body = transformStatements(node->body);
}

void StatementsVisitor::onFinishVisit(IfStatement* node) {
    if (auto ifStatement = dynamic_cast<IfStatement*>(node->elseStatement)) {
        node->elseStatement = transformIfStatement(ifStatement);
    }
}

void StatementsVisitor::onStartVisit(BlockStatement *node) {
    node->body = transformStatementsWithContinues(node->body);
}

void StatementsVisitor::onStartVisit(ForStatement* node) {
    nextIterationsLoops.push(node->iterationStatement);
}

void StatementsVisitor::onFinishVisit(ForStatement* node) {
    nextIterationsLoops.pop();
}

void StatementsVisitor::onStartVisit(WhileStatement* node) {
    nextIterationsLoops.push(nullptr);
}

void StatementsVisitor::onFinishVisit(WhileStatement* node) {
    nextIterationsLoops.pop();
}

void StatementsVisitor::onStartVisit(ForRangeStatement* node) {
    StatementAST* nextIteration = new ExpressionStatement(
        new UnaryExpression(
            UnaryExpressionEnum::Increment,
            new IdentifierAsExpression(indexPrivateVariableName))
        );
    nextIterationsLoops.push(nextIteration);
}

void StatementsVisitor::onFinishVisit(ForRangeStatement* node) {
    nextIterationsLoops.pop();
}

void StatementsVisitor::transform(PackageAST* packageAst) {
    packageAst->acceptVisitor(this);
}

void StatementsVisitor::onStartVisit(SwitchCaseClause *node) {
    node->statementsList = transformStatementsWithContinues(node->statementsList);
}

void StatementsVisitor::onStartVisit(SwitchStatement *node) {
    node->defaultStatement = transformStatementsWithContinues(node->defaultStatement);
}

void StatementsVisitor::onFinishVisit(SwitchCaseClause* node) {
    node->statementsList = transformStatements(node->statementsList);
}

void StatementsVisitor::onFinishVisit(SwitchStatement* node) {
    node->defaultStatement = transformStatements(node->defaultStatement);
}

bool StatementsVisitor::checkReturnStatements(StatementAST* stmt) {

    if (auto returnStmt = dynamic_cast<ReturnStatement*>(stmt)) {
        return true;

    } else if (auto ifStmt = dynamic_cast<IfStatement*>(stmt)) {
        return checkReturnStatements(ifStmt);

    } else if (auto block = dynamic_cast<BlockStatement*>(stmt)) {
        return checkReturnStatements(block);

    } else if (auto switchStmt = dynamic_cast<SwitchStatement*>(stmt)) {
        return checkReturnStatements(switchStmt);
    }

    return false;
}

bool StatementsVisitor::checkReturnStatements(BlockStatement* block) {
    if (block->body.size() != 0) {
        return checkReturnStatements(*(--block->body.end()));
    }
    return false;
}

bool StatementsVisitor::checkReturnStatements(IfStatement* ifStatement) {
    bool hasReturnInThenBlock = ifStatement->thenStatement->body.size() != 0? 
            checkReturnStatements(*(--ifStatement->thenStatement->body.end())) : false;

    if (ifStatement->elseStatement != nullptr) {
        return hasReturnInThenBlock && checkReturnStatements(ifStatement->elseStatement);
    }

    return false;
}

bool StatementsVisitor::checkReturnStatements(SwitchStatement* switchStmt) {
    bool casesHaveReturn = switchStmt->defaultStatement.size() != 0? 
            checkReturnStatements(*(--switchStmt->defaultStatement.end())) : false;

    for (auto caseClause : switchStmt->clauseList) {

        if (caseClause->statementsList.size() != 0) {
            casesHaveReturn &= checkReturnStatements(*(--caseClause->statementsList.end()));

        } else {
            casesHaveReturn = false;
        }
    }

    return casesHaveReturn;
}

void StatementsVisitor::onFinishVisit(FunctionDeclaration* node) {
    if (node->signature->idsAndTypesResults.size() != 0 && !checkReturnStatements(node->block)) {

        semantic->errors.push_back("Missing the 'return' statement at the end of the function");
    }
}
