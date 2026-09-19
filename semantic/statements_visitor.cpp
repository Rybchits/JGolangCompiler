#include "statements_visitor.h"
#include "../utils/clone_visitor.h"

const std::string StatementsVisitor::indexPrivateVariableName = "$index";

void StatementsVisitor::transformAssignment(AssignmentStatement* assignment) {
    if (assignment->type == AssignmentStatement::SimpleAssign) return;

    BinaryExpression::BinaryExpressionEnum operation;
    switch (assignment->type) {
        case AssignmentStatement::MinusAssign: operation = BinaryExpression::Subtraction; break;
        case AssignmentStatement::PlusAssign: operation = BinaryExpression::Addition; break;
        case AssignmentStatement::MulAssign: operation = BinaryExpression::Multiplication; break;
        case AssignmentStatement::DivAssign: operation = BinaryExpression::Division; break;
        case AssignmentStatement::ModAssign: operation = BinaryExpression::Mod; break;
        default: return;
    }
    auto leftValue = cloneNode(*assignment->lhs.front());
    if (auto* identifier = dynamic_cast<IdentifierAsExpression*>(leftValue.get())) {
        identifier->isDestination = false;
    }
    assignment->rhs.front() = std::make_unique<BinaryExpression>(
        operation, std::move(leftValue), std::move(assignment->rhs.front()));
    assignment->type = AssignmentStatement::SimpleAssign;
}

BlockStatementPtr StatementsVisitor::transformForToWhile(ForStatement* forStmt) {
    StatementList list;
    if (forStmt->initStatement) list.push_back(std::move(forStmt->initStatement));

    auto whileLoop = std::make_unique<WhileStatement>(
        forStmt->conditionExpression ? std::move(forStmt->conditionExpression) : std::make_unique<BooleanExpression>(true),
        std::move(forStmt->block));
    if (forStmt->iterationStatement) {
        whileLoop->block->body.push_back(std::move(forStmt->iterationStatement));
    }
    list.push_back(std::move(whileLoop));
    return std::make_unique<BlockStatement>(std::move(list));
}

StatementASTPtr StatementsVisitor::transformIfStatement(StatementASTPtr stmt) {
    auto* ifStmt = static_cast<IfStatement*>(stmt.get());
    if (!ifStmt->preStatement) return stmt;
    StatementList body;
    body.push_back(std::move(ifStmt->preStatement));
    body.push_back(std::move(stmt));
    return std::make_unique<BlockStatement>(std::move(body));
}

StatementASTPtr StatementsVisitor::transformSwitchStatement(StatementASTPtr stmt) {
    auto* switchStmt = static_cast<SwitchStatement*>(stmt.get());
    if (!switchStmt->statement) return stmt;
    StatementList body;
    body.push_back(std::move(switchStmt->statement));
    body.push_back(std::move(stmt));
    return std::make_unique<BlockStatement>(std::move(body));
}

// Перед каждым Continue добавить statement перехода
StatementList StatementsVisitor::transformKeywordStatements(StatementList body) {
    StatementList newBody;

    for (auto& stmt: body) {
        auto keyword = dynamic_cast<KeywordStatement *>(stmt.get());

        if (keyword) {
            switch (keyword->type)
            {
                case KeywordStatement::Continue:
                    if (nextIterationsLoops.empty()) {
                        addError("Continue keyword out of loop");
                        
                    } else if (nextIterationsLoops.top() != nullptr) {
                        newBody.push_back(cloneNode(*nextIterationsLoops.top()));
                    }
                    break;

                case KeywordStatement::Break:
                    if (nextIterationsLoops.empty() && !insideSwitchCaseClause) {
                        addError("Break keyword out of loop and switch case clause");
                    }
                    break;

                case KeywordStatement::Fallthrough:
                    addError("Fallthrough keyword out of maswitch case clause");
                    break;
            }
        }

        newBody.push_back(std::move(stmt));
    }
    return newBody;
}

BlockStatementPtr StatementsVisitor::transformForRangeToWhile(ForRangeStatement *forRangeStmt) {
    StatementList list;

    auto indexDeclaration = std::make_unique<VariableDeclaration>(
            std::make_unique<IdentifiersWithType>(IdentifiersList{indexPrivateVariableName}, std::make_unique<IdentifierAsType>("int")),
            MakeList<ExpressionList>(std::make_unique<IntegerExpression>(0))
    );

    list.push_back(std::make_unique<DeclarationStatement>(MakeList<DeclarationList>(std::move(indexDeclaration))));

    auto condition = std::make_unique<BinaryExpression>(
            BinaryExpression::Less,
            std::make_unique<IdentifierAsExpression>(indexPrivateVariableName),
            std::make_unique<CallableExpression>(std::make_unique<IdentifierAsExpression>("len"), MakeList<ExpressionList>(cloneNode(*forRangeStmt->expressionValue)))
    );

    if (forRangeStmt->initStatement.size() > 2) {
        addError("Many variables in initialization ForRange loop");
    }

    auto variableForRange = forRangeStmt->initStatement.begin();

    // Index variable
    if (forRangeStmt->initStatement.size() >= 1) {
        StatementASTPtr indexVariableStatement;

        if (forRangeStmt->hasShortDeclaration) {
            if (auto indexVariableIdentifier = dynamic_cast<IdentifierAsExpression *>(variableForRange->get())) {
                indexVariableStatement = std::make_unique<DeclarationStatement>(
                        std::make_unique<VariableDeclaration>(
                                std::make_unique<IdentifiersWithType>(indexVariableIdentifier->identifier, nullptr),
                                MakeList<ExpressionList>(std::make_unique<IdentifierAsExpression>(indexPrivateVariableName))
                        )
                );
            } else {
                addError("Undefined expression (1) in initialization ForRange loop");
            }
        } else {
            indexVariableStatement = std::make_unique<AssignmentStatement>(
                    AssignmentStatement::SimpleAssign,
                    std::move(*variableForRange),
                    std::make_unique<IdentifierAsExpression>(indexPrivateVariableName));
        }

        if (indexVariableStatement) forRangeStmt->block->body.push_front(std::move(indexVariableStatement));
    }

    // Element variable
    if (forRangeStmt->initStatement.size() >= 2) {
        StatementASTPtr elementVariableStatement;
        variableForRange++;

        auto accessToElement = std::make_unique<AccessExpression>(
                AccessExpression::Indexing,
                std::move(forRangeStmt->expressionValue),
                std::make_unique<IdentifierAsExpression>(indexPrivateVariableName)
        );

        if (forRangeStmt->hasShortDeclaration) {
            if (auto elementVariableIdentifier = dynamic_cast<IdentifierAsExpression *>(variableForRange->get())) {
                elementVariableStatement = std::make_unique<DeclarationStatement>(
                        std::make_unique<VariableDeclaration>(
                                std::make_unique<IdentifiersWithType>(elementVariableIdentifier->identifier, nullptr),
                                MakeList<ExpressionList>(std::move(accessToElement))
                        )
                );
            } else {
                addError("Undefined expression (2) in initialization ForRange loop");
            }
        } else {
            elementVariableStatement = std::make_unique<AssignmentStatement>(
                    AssignmentStatement::SimpleAssign,
                    std::move(*variableForRange),
                    std::move(accessToElement));
        }

        if (elementVariableStatement) forRangeStmt->block->body.push_front(std::move(elementVariableStatement));
    }

    forRangeStmt->block->body.insert(forRangeStmt->block->body.end(), std::make_unique<ExpressionStatement>(
            std::make_unique<UnaryExpression>(
                    UnaryExpression::Increment,
                    std::make_unique<IdentifierAsExpression>(indexPrivateVariableName))));

    list.push_back(std::make_unique<WhileStatement>(std::move(condition), std::move(forRangeStmt->block)));
    return std::make_unique<BlockStatement>(std::move(list));
}

StatementList StatementsVisitor::transformStatements(StatementList list) {
    for (auto& stmt : list) {
        if (auto* forLoop = dynamic_cast<ForStatement*>(stmt.get())) {
            stmt = transformForToWhile(forLoop);
        } else if (auto* rangeLoop = dynamic_cast<ForRangeStatement*>(stmt.get())) {
            stmt = transformForRangeToWhile(rangeLoop);
        } else if (dynamic_cast<IfStatement*>(stmt.get())) {
            stmt = transformIfStatement(std::move(stmt));
        } else if (dynamic_cast<SwitchStatement*>(stmt.get())) {
            stmt = transformSwitchStatement(std::move(stmt));
        } else if (auto* assignment = dynamic_cast<AssignmentStatement*>(stmt.get())) {
            transformAssignment(assignment);
        }
    }
    return list;
}

void StatementsVisitor::onFinishVisit(BlockStatement& node) {
    node.body = transformStatements(std::move(node.body));
}

void StatementsVisitor::onFinishVisit(IfStatement& node) {
    if (auto ifStatement = dynamic_cast<IfStatement*>(node.elseStatement.get())) {
        node.elseStatement = transformIfStatement(std::move(node.elseStatement));
    }
}

void StatementsVisitor::onStartVisit(BlockStatement& node) {
    node.body = transformKeywordStatements(std::move(node.body));
}

void StatementsVisitor::onStartVisit(ForStatement& node) {
    nextIterationsLoops.push(node.iterationStatement ? cloneNode(*node.iterationStatement) : nullptr);
}

void StatementsVisitor::onFinishVisit(ForStatement& node) {
    nextIterationsLoops.pop();
}

void StatementsVisitor::onStartVisit(WhileStatement& node) {
    nextIterationsLoops.push(nullptr);
}

void StatementsVisitor::onFinishVisit(WhileStatement& node) {
    nextIterationsLoops.pop();
}

void StatementsVisitor::onStartVisit(ForRangeStatement& node) {
    auto nextIteration = std::make_unique<ExpressionStatement>(
        std::make_unique<UnaryExpression>(
            UnaryExpression::Increment,
            std::make_unique<IdentifierAsExpression>(indexPrivateVariableName))
        );
    nextIterationsLoops.push(std::move(nextIteration));
}

void StatementsVisitor::onFinishVisit(ForRangeStatement& node) {
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
        return checkReturnStatements(block->body.back().get());
    }
    return false;
}

bool StatementsVisitor::checkReturnStatements(IfStatement* ifStatement) {
    bool hasReturnInThenBlock = ifStatement->thenStatement->body.size() != 0? 
            checkReturnStatements(ifStatement->thenStatement->body.back().get()) : false;

    if (ifStatement->elseStatement != nullptr) {
        return hasReturnInThenBlock && checkReturnStatements(ifStatement->elseStatement.get());
    }

    return false;
}

bool StatementsVisitor::checkReturnStatements(SwitchStatement* switchStmt) {
    bool casesHaveReturn = true;
    bool hasDefault = false;

    for (const auto& caseClause : switchStmt->clauseList) {
        if (caseClause->expressionCase == nullptr) {
            hasDefault = true;
        }
        casesHaveReturn &= checkReturnStatements(caseClause->block.get());
    }

    return casesHaveReturn && hasDefault;
}

void StatementsVisitor::onFinishVisit(FunctionDeclaration& node) {
    if (node.signature->idsAndTypesResults.size() != 0 && !checkReturnStatements(node.block.get())) {

        addError("Missing the 'return' statement at the end of the function");
    }
}

void StatementsVisitor::onStartVisit(SwitchCaseClause& node) {
    insideSwitchCaseClause = true;

    if (node.block->body.size() != 0) {
        auto keyword = dynamic_cast<KeywordStatement*>(node.block->body.back().get());

        if (keyword && keyword->type == KeywordStatement::Fallthrough) {
            node.fallthrowEnds = true;
            node.block->body.pop_back();
        }
    }
}

void StatementsVisitor::onFinishVisit(SwitchStatement& node) {
    bool hasDefault = false;

    int index = 0;
    for (const auto& caseClause : node.clauseList) {

        if (hasDefault && caseClause->expressionCase == nullptr) {
            addError("Switch has multiple defaults");
        }

        if (index == node.clauseList.size() - 1 && caseClause->fallthrowEnds) {
            addError("Last case can't end with fallthrough");
        }

        hasDefault |= caseClause->expressionCase == nullptr;
        index++;
    }
}

void StatementsVisitor::onFinishVisit(SwitchCaseClause& node) {
    insideSwitchCaseClause = false;
}
