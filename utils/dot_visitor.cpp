#include "dot_visitor.h"

bool DotConvertVisitor::hasConnection(size_t id1, size_t id2) {
    return std::find(connections[id1].begin(), connections[id1].end(), id2) != connections[id1].end();
}

[[nodiscard]] std::string DotConvertVisitor::MakeNode(const std::size_t id, const std::string_view name) {
    return "Id" + std::to_string(id) + " [label=\"" + std::string{name} + "\"]\n";
}

[[nodiscard]] std::string DotConvertVisitor::MakeConnection(const size_t id1, const size_t id2, std::string_view note) {
    std::string res = "";

    if (!hasConnection(id1, id2)) {
        res = "Id" + std::to_string(id1) + " -> " + "Id" + std::to_string(id2);

        if (!note.empty())
            res += " [label=\"" + std::string{note} + "\"]";

        res += "\n";
        connections[id1].push_back(id2);
    }
    
    return res;
}

void DotConvertVisitor::convert(NodeAST *node) {
    out << "digraph Program {\n";
    node->acceptVisitor(this);
    out << "}" << std::endl;
}

void DotConvertVisitor::onStartVisit(PackageAST *node) {
    out << MakeNode(node->nodeId, node->name() + " " + node->packageName);
    for (auto decl: node->topDeclarations) {
        out << MakeConnection(node->nodeId, decl->nodeId);
    }
}

void DotConvertVisitor::onStartVisit(VariableDeclaration *node) {
    out << MakeNode(node->nodeId, (node->isConst ? "Const " : "") + node->name());

    int index = 0;
    for (const auto &value: node->values) {
        out << MakeConnection(node->nodeId, value->nodeId, std::to_string(index++));
    }

    out << MakeConnection(node->nodeId, node->identifiersWithType->nodeId, "ids");
}

void DotConvertVisitor::onStartVisit(TypeDeclaration *node) {
    out << MakeNode(node->nodeId, node->name() + ": " + node->alias);
    out << MakeConnection(node->nodeId, node->declType->nodeId);
}

void DotConvertVisitor::onStartVisit(FunctionDeclaration *node) {
    out << MakeNode(node->nodeId, node->name() + " " + node->identifier);

    out << MakeConnection(node->nodeId, node->signature->nodeId);

    if (node->block != nullptr) {
        out << MakeConnection(node->nodeId, node->block->nodeId);
    }
}

void DotConvertVisitor::onStartVisit(MethodDeclaration *node) {
    out << MakeNode(node->nodeId, node->name() + " " + node->identifier);
    out << MakeConnection(node->nodeId, node->receiverType->nodeId, "receiver");
    out << MakeConnection(node->nodeId, node->signature->nodeId);
    out << MakeConnection(node->nodeId, node->block->nodeId);
}

void DotConvertVisitor::onStartVisit(IdentifierAsExpression *node) {
    out << MakeNode(node->nodeId, "Id: " + node->identifier);
}

void DotConvertVisitor::onStartVisit(BooleanExpression *node) {
    out << MakeNode(node->nodeId, "Bool: " + std::to_string(node->boolLit));
}

void DotConvertVisitor::onStartVisit(FloatExpression *node) {
    out << MakeNode(node->nodeId, "Float: " + std::to_string(node->floatLit));
}

void DotConvertVisitor::onStartVisit(StringExpression *node) {
    out << MakeNode(node->nodeId, "String: " + node->stringLit);
}

void DotConvertVisitor::onStartVisit(RuneExpression *node) {
    out << MakeNode(node->nodeId, "Rune: " + std::to_string(node->runeLit));
}

void DotConvertVisitor::onStartVisit(NilExpression *node) {
    out << MakeNode(node->nodeId, "Nil");
}

void DotConvertVisitor::onStartVisit(FunctionLitExpression *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->signature->nodeId);
    out << MakeConnection(node->nodeId, node->block->nodeId);
}

void DotConvertVisitor::onStartVisit(UnaryExpression *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->expression->nodeId);
}

void DotConvertVisitor::onStartVisit(BinaryExpression *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->lhs->nodeId, "left");
    out << MakeConnection(node->nodeId, node->rhs->nodeId, "right");
}

void DotConvertVisitor::onStartVisit(CallableExpression *node) {
    out << MakeNode(node->nodeId, node->name());

    out << MakeConnection(node->nodeId, node->base->nodeId, "base");

    int index = 0;
    for (auto arg: node->arguments) {
        out << MakeConnection(node->nodeId, arg->nodeId, std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(AccessExpression *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->base->nodeId, "base");
    out << MakeConnection(node->nodeId, node->accessor->nodeId, "access");
}

void DotConvertVisitor::onStartVisit(BlockStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    int index = 0;
    for (auto stmt: node->body) {
        out << MakeConnection(node->nodeId, stmt->nodeId, std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(KeywordStatement *node) {
    out << MakeNode(node->nodeId, node->name());
}

void DotConvertVisitor::onStartVisit(ExpressionStatement *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->expression->nodeId);
}

void DotConvertVisitor::onStartVisit(AssignmentStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    int index = 0;
    auto indexIterator = node->indexes.begin();
    for (auto leftExpr: node->lhs) {
        out << MakeConnection(node->nodeId, leftExpr->nodeId, "l" + std::to_string(index));

        if (*indexIterator != nullptr)
            out << MakeConnection(node->nodeId, (*indexIterator)->nodeId, "index " + std::to_string(index));

        index++;
        indexIterator++;
    }

    index = 0;
    for (auto rightExpr: node->rhs) {
        out << MakeConnection(node->nodeId, rightExpr->nodeId, "r" + std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(ForStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    if (node->initStatement != nullptr) {
        out << MakeConnection(node->nodeId, node->initStatement->nodeId, "init");
    }

    if (node->conditionExpression != nullptr) {
        out << MakeConnection(node->nodeId, node->conditionExpression->nodeId, "cond");
    }

    if (node->iterationStatement != nullptr) {
        out << MakeConnection(node->nodeId, node->iterationStatement->nodeId, "next");
    }

    out << MakeConnection(node->nodeId, node->block->nodeId, "block");
}

void DotConvertVisitor::onStartVisit(WhileStatement *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->conditionExpression->nodeId, "cond");
    out << MakeConnection(node->nodeId, node->block->nodeId, "block");
}

void DotConvertVisitor::onStartVisit(ForRangeStatement *node) {
    out << MakeNode(node->nodeId, node->name() + (node->hasShortDeclaration ? " with := " : ""));

    int index = 0;
    for (auto initStmt: node->initStatement) {
        out << MakeConnection(node->nodeId, initStmt->nodeId, std::to_string(index));
    }
    out << MakeConnection(node->nodeId, node->expressionValue->nodeId);
    out << MakeConnection(node->nodeId, node->block->nodeId, "block");
}

void DotConvertVisitor::onStartVisit(ReturnStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    int index = 0;
    for (auto expr: node->returnValues) {
        out << MakeConnection(node->nodeId, expr->nodeId, std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(IfStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    if (node->preStatement != nullptr) {
        out << MakeConnection(node->nodeId, node->preStatement->nodeId, "init");
    }

    out << MakeConnection(node->nodeId, node->condition->nodeId, "condition");
    out << MakeConnection(node->nodeId, node->thenStatement->nodeId, "then");

    if (node->elseStatement != nullptr) {
        out << MakeConnection(node->nodeId, node->elseStatement->nodeId, "else");
    }
}

void DotConvertVisitor::onStartVisit(SwitchCaseClause *node) {
    out << MakeNode(node->nodeId, node->name());
    out << MakeConnection(node->nodeId, node->expressionCase->nodeId, "key");

    out << MakeConnection(node->nodeId, node->block->nodeId, "block");
}

void DotConvertVisitor::onStartVisit(SwitchStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    if (node->statement != nullptr) {
        out << MakeConnection(node->nodeId, node->statement->nodeId, "init");
    }

    if (node->expression != nullptr) {
        out << MakeConnection(node->nodeId, node->expression->nodeId, "expr");
    }

    int index = 0;
    for (auto caseClause: node->clauseList) {

        out << MakeConnection(node->nodeId, caseClause->nodeId, 
            caseClause->expressionCase? "case " : "default " + std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(DeclarationStatement *node) {
    out << MakeNode(node->nodeId, node->name());

    for (auto decl: node->declarations) {
        out << MakeConnection(node->nodeId, decl->nodeId);
    }
}

void DotConvertVisitor::onStartVisit(IdentifiersWithType *node) {
    std::string label;

    int index = 0;
    for (const auto &id: node->identifiers) {
        label += std::to_string(index++) + ": " + id + '\n';
    }
    out << MakeNode(node->nodeId, label);

    if (node->type != nullptr) {
        out << MakeConnection(node->nodeId, node->type->nodeId, "type");
    }
}

void DotConvertVisitor::onStartVisit(FunctionSignature *node) {
    out << MakeNode(node->nodeId, node->name());

    int index = 0;
    for (auto arg: node->idsAndTypesArgs) {
        out << MakeConnection(node->nodeId, arg->nodeId, "arg" + std::to_string(index++));
    }

    index = 0;
    for (auto res: node->idsAndTypesResults) {
        out << MakeConnection(node->nodeId, res->nodeId, "res" + std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(ArraySignature *node) {
    std::string label =
            node->dimensions == -1 ? "TypeSlice" : node->name() + "[" + std::to_string(node->dimensions) + "]";
    out << MakeNode(node->nodeId, label);
    out << MakeConnection(node->nodeId, node->arrayElementType->nodeId);
}

void DotConvertVisitor::onStartVisit(StructSignature *node) {
    out << MakeNode(node->nodeId, node->name());
    for (auto member: node->structMembers) {
        std::string connection;
        if (member->identifiers.size() == 1 && member->identifiers.front().empty()) {
            connection = "composition";
        }

        out << MakeConnection(node->nodeId, member->nodeId, connection);
    }
}

void DotConvertVisitor::onStartVisit(IdentifierAsType *node) {
    std::string label = "Type: " + node->identifier;

    if (node->isVariadic)
        label += "\n Variadic";

    if (node->isPointer)
        label += "\n Pointer";

    out << MakeNode(node->nodeId, label);
}

void DotConvertVisitor::onStartVisit(NodeAST *node) {}

void DotConvertVisitor::onStartVisit(InterfaceType *node) {
    out << MakeNode(node->nodeId, node->name());

    for (auto func: node->functions) {
        out << MakeConnection(node->nodeId, func->nodeId);
    }
}

void DotConvertVisitor::onStartVisit(CompositeLiteral *node) {
    out << MakeNode(node->nodeId, node->name());

    out << MakeConnection(node->nodeId, node->type->nodeId);

    int index = 0;
    for (auto element : node->elements) {
        out << MakeConnection(node->nodeId, element->nodeId, std::to_string(index++));
    }
}

void DotConvertVisitor::onStartVisit(ElementCompositeLiteral *node) {
    out << MakeNode(node->nodeId, "Element");

    if (node->key != nullptr) {
        out << MakeConnection(node->nodeId, node->key->nodeId, "Key");
    }

    if (std::holds_alternative<ExpressionAST *>(node->value)) {
        auto temp = std::get<ExpressionAST *>(node->value);
        out << MakeConnection(node->nodeId, temp->nodeId);
    } else if (std::holds_alternative<ElementCompositeLiteralList>(node->value)) {
        int index = 0;
        for (auto insideElement : std::get<ElementCompositeLiteralList>(node->value)) {
            out << MakeConnection(node->nodeId, insideElement->nodeId, std::to_string(index++));
        }
    }
}

void DotConvertVisitor::onStartVisit(IntegerExpression *node) {
    out << MakeNode(node->nodeId, "Int: " + std::to_string(node->intLit));
}

void DotConvertVisitor::onStartVisit(ShortVarDeclarationStatement *node) {
    std::string label = node->name() + "\n";

    int index = 0;
    for (const auto &id: node->identifiers) {
        label += std::to_string(index++) + ": " + id + '\n';
    }
    out << MakeNode(node->nodeId, label);

    for (const auto &exp: node->values) {
        out << MakeConnection(node->nodeId, exp->nodeId);
    }
}