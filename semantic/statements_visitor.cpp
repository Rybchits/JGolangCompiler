#include "statements_visitor.h"

const std::string StatementsVisitor::indexPrivateVariableName = "$index";

AssignmentStatement* StatementsVisitor::transformAssignment(AssignmentStatement* assigmnent) {
    if (assigmnent->type != AssignmentStatement::SimpleAssign) {
        auto leftValueExpression = assigmnent->lhs.front();
        auto rightValueExpression = assigmnent->rhs.front();

        switch (assigmnent->type)
        {
        case AssignmentStatement::MinusAssign:
            assigmnent->rhs = ExpressionList({new BinaryExpression(
                BinaryExpression::Subtraction, leftValueExpression->clone(), rightValueExpression)});
            break;

        case AssignmentStatement::PlusAssign:
            assigmnent->rhs = ExpressionList({new BinaryExpression(
                BinaryExpression::Addition, leftValueExpression->clone(), rightValueExpression)});
            break;

        case AssignmentStatement::MulAssign:
            assigmnent->rhs = ExpressionList({new BinaryExpression(
                BinaryExpression::Multiplication, leftValueExpression->clone(), rightValueExpression)});
            break;

        case AssignmentStatement::DivAssign:
            assigmnent->rhs = ExpressionList({new BinaryExpression(
                BinaryExpression::Division, leftValueExpression->clone(), rightValueExpression)});
            break;

        case AssignmentStatement::ModAssign:
            assigmnent->rhs = ExpressionList({new BinaryExpression(
                BinaryExpression::Mod, leftValueExpression->clone(), rightValueExpression)});
            break;
        
        default:
            break;
        }
    }
    
    assigmnent->type = AssignmentStatement::SimpleAssign;
    return assigmnent;
}

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
StatementList StatementsVisitor::transformKeywordStatements(StatementList body) {
    StatementList newBody;

    for (auto stmt: body) {
        auto keyword = dynamic_cast<KeywordStatement *>(stmt);

        if (keyword) {
            switch (keyword->type)
            {
                case KeywordStatement::Continue:
                    if (nextIterationsLoops.empty()) {
                        semantic->addError("Continue keyword out of loop");
                        
                    } else if (nextIterationsLoops.top() != nullptr) {
                        newBody.push_back(nextIterationsLoops.top()->clone());
                    }
                    break;

                case KeywordStatement::Break:
                    if (nextIterationsLoops.empty() && !insideSwitchCaseClause) {
                        semantic->addError("Break keyword out of loop and switch case clause");
                    }
                    break;

                case KeywordStatement::Fallthrough:
                    semantic->addError("Fallthrough keyword out of maswitch case clause");
                    break;
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
            BinaryExpression::Less,
            new IdentifierAsExpression(indexPrivateVariableName),
            new CallableExpression(new IdentifierAsExpression("len"),
                                   *(new ExpressionList{forRangeStmt->expressionValue->clone()}))
    );

    if (forRangeStmt->initStatement.size() > 2) {
        semantic->addError("Many variables in initialization ForRange loop");
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
                semantic->addError("Undefined expression (1) in initialization ForRange loop");
            }
        } else {
            indexVariableStatement = new AssignmentStatement(
                    AssignmentStatement::SimpleAssign,
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
                AccessExpression::Indexing,
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
                semantic->addError("Undefined expression (2) in initialization ForRange loop");
            }
        } else {
            elementVariableStatement = new AssignmentStatement(
                    AssignmentStatement::SimpleAssign,
                    *variableForRange,
                    accessToElement);
        }

        forRangeStmt->block->body.insert(forRangeStmt->block->body.begin(), elementVariableStatement);
    }

    forRangeStmt->block->body.insert(forRangeStmt->block->body.end(), new ExpressionStatement(
            new UnaryExpression(
                    UnaryExpression::Increment,
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

        } else if (auto assignmentStmt = dynamic_cast<AssignmentStatement*>(stmt)) {
            newBody.push_back(transformAssignment(assignmentStmt));

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
    node->body = transformKeywordStatements(node->body);
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
            UnaryExpression::Increment,
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
    bool casesHaveReturn = true;
    bool hasDefault = false;

    for (auto caseClause : switchStmt->clauseList) {
        if (caseClause->expressionCase == nullptr) {
            hasDefault = true;
        }
        casesHaveReturn &= checkReturnStatements(caseClause->block);
    }

    return casesHaveReturn && hasDefault;
}

void StatementsVisitor::onFinishVisit(FunctionDeclaration* node) {
    if (node->signature->idsAndTypesResults.size() != 0 && !checkReturnStatements(node->block)) {

        semantic->addError("Missing the 'return' statement at the end of the function");
    }
}

void StatementsVisitor::onStartVisit(SwitchCaseClause* node) {
    insideSwitchCaseClause = true;

    if (node->block->body.size() != 0) {
        auto keyword = dynamic_cast<KeywordStatement*>(node->block->body.back());

        if (keyword && keyword->type == KeywordStatement::Fallthrough) {
            node->fallthrowEnds = true;
            node->block->body.pop_back();
        }
    }
}

void StatementsVisitor::onFinishVisit(SwitchStatement* node) {
    bool hasDefault = false;

    int index = 0;
    for (auto caseClause : node->clauseList) {

        if (hasDefault && caseClause->expressionCase == nullptr) {
            semantic->addError("Switch has multiple defaults");
        }

        if (index == node->clauseList.size() - 1 && caseClause->fallthrowEnds) {
            semantic->addError("Last case can't end with fallthrough");
        }

        hasDefault |= caseClause->expressionCase == nullptr;
        index++;
    }
}

void StatementsVisitor::onFinishVisit(SwitchCaseClause* node) {
    insideSwitchCaseClause = false;
}
