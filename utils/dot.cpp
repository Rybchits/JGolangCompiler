#include <sstream>
#include "../ast.h"

[[nodiscard]] std::string MakeNode(const std::size_t id, const std::string_view name)
{
    return "Id" + std::to_string(id) + " [label=\"" + std::string{ name } + "\"]\n";
}

[[nodiscard]] std::string MakeConnection(const size_t id1, const size_t id2, std::string_view note = "")
{
    auto res = "Id" + std::to_string(id1) + " -> " + "Id" + std::to_string(id2);

    if (!note.empty())
        res += " [label=\"" + std::string{ note } + "\"]";

    res += "\n";
    return res;
}

void PackageAST::toDot(std::ostream& out) const noexcept {
    out << "digraph Program {\n";
    out << MakeNode(nodeId, name() + " " + packageName);
    for (auto decl : *topDeclarations) {
        out << MakeConnection(nodeId, decl->nodeId);
        decl->toDot(out);
    }
    out << "}" << std::endl;
}

void FunctionSignature::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    int index = 0;
    for (auto arg : idsAndTypesArgs) {
        out << MakeConnection(nodeId, arg->nodeId, "arg" + std::to_string(index++));
        arg->toDot(out);
    }

    index = 0;
    for (auto res : idsAndTypesResults) {
        out << MakeConnection(nodeId, res->nodeId, "res" + std::to_string(index++));
        res->toDot(out);
    }
}

void StructSignature::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());
    for (auto member : structMembers) {
        std::string connection;
        if (member->identifiers.size() == 1 && member->identifiers.front() == "") {
            connection = "composition";
        }

        out << MakeConnection(nodeId, member->nodeId, connection);
        member->toDot(out);
    }
}

void ArraySignature::toDot(std::ostream &out) const noexcept {
    std::string label = dimensions == -1? "TypeSlice" : name() + "[" + std::to_string(dimensions) + "]";
    out << MakeNode(nodeId, label);

    out << MakeConnection(nodeId, arrayElementType->nodeId);
    arrayElementType->toDot(out);
}

void IdentifiersWithType::toDot(std::ostream &out) const noexcept {
    std::string label;

    int index = 0;
    for (const auto& id : identifiers) {
        label += std::to_string(index++) + ": " + id + '\n';
    }

    out << MakeNode(nodeId, label);

    if (type != nullptr) {
        out << MakeConnection(nodeId, type->nodeId, "type");
        type->toDot(out);
    }
}

void DeclarationStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    for (auto decl : declarations) {
        out << MakeConnection(nodeId, decl->nodeId);
        decl->toDot(out);
    }
}

void SwitchStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    if (statement != nullptr) {
        out << MakeConnection(nodeId, statement->nodeId, "init");
        statement->toDot(out);
    }

    if (expression != nullptr) {
        out << MakeConnection(nodeId, expression->nodeId, "expr");
        expression->toDot(out);
    }

    int index = 0;
    for (auto caseClause : clauseList) {
        out << MakeConnection(nodeId, caseClause->nodeId, "case " + std::to_string(index++));
        caseClause->toDot(out);
    }

    index = 0;
    for (auto defaultStmt : defaultStatement) {
        out << MakeConnection(nodeId, defaultStmt->nodeId, "default " + std::to_string(index++));
        defaultStmt->toDot(out);
    }
}

void SwitchCaseClause::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    out << MakeConnection(nodeId, expressionCase->nodeId, "key");
    expressionCase->toDot(out);

    int index = 0;
    for (auto stmt : statementsList) {
        out << MakeConnection(nodeId, stmt->nodeId, std::to_string(index++));
        stmt->toDot(out);
    }
}

void IfStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    if (preStatement != nullptr) {
        out << MakeConnection(nodeId, preStatement->nodeId, "init");
        preStatement->toDot(out);
    }

    out << MakeConnection(nodeId, condition->nodeId, "condition");
    condition->toDot(out);

    out << MakeConnection(nodeId, thenStatement->nodeId, "then");
    thenStatement->toDot(out);

    if (elseStatement != nullptr) {
        out << MakeConnection(nodeId, elseStatement->nodeId, "else");
        elseStatement->toDot(out);
    }
}

void ReturnStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    int index = 0;
    for (auto expr : returnValues) {
        out << MakeConnection(nodeId, expr->nodeId, std::to_string(index++));
        expr->toDot(out);
    }
}

void ForRangeStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name() + ( hasShortDeclaration? " with := " : ""));

    int index = 0;
    for (auto initStmt : initStatement) {
        out << MakeConnection(nodeId, initStmt->nodeId, std::to_string(index));
        initStmt->toDot(out);
    }

    out << MakeConnection(nodeId, expressionValue->nodeId);
    expressionValue->toDot(out);

    out << MakeConnection(nodeId, block->nodeId, "block");
    block->toDot(out);
}

void WhileStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    out << MakeConnection(nodeId, conditionExpression->nodeId, "cond");
    conditionExpression->toDot(out);

    out << MakeConnection(nodeId, block->nodeId, "block");
    block->toDot(out);
}

void ForStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    if (initStatement != nullptr) {
        out << MakeConnection(nodeId, initStatement->nodeId, "init");
        initStatement->toDot(out);
    }

    if (conditionExpression != nullptr) {
        out << MakeConnection(nodeId, conditionExpression->nodeId, "cond");
        conditionExpression->toDot(out);
    }

    if (iterationStatement != nullptr) {
        out << MakeConnection(nodeId, iterationStatement->nodeId, "next");
        iterationStatement->toDot(out);
    }

    out << MakeConnection(nodeId, block->nodeId, "block");
    block->toDot(out);
}

void AssignmentStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    int index = 0;
    for (auto leftExpr : lhs) {
        out << MakeConnection(nodeId, leftExpr->nodeId, std::to_string(index++));
        leftExpr->toDot(out);
    }

    index = 0;
    for (auto rightExpr : rhs) {
        out << MakeConnection(nodeId, rightExpr->nodeId, std::to_string(index++));
        rightExpr->toDot(out);
    }
}

void ExpressionStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());
    out << MakeConnection(nodeId, expression->nodeId);
    expression->toDot(out);
}

void KeywordStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());
}

void BlockStatement::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    int index = 0;
    for(auto stmt : body) {
        out << MakeConnection(nodeId, stmt->nodeId, std::to_string(index++));
        stmt->toDot(out);
    }
}

void CompositeLiteral::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    out << MakeConnection(nodeId, type->nodeId);
    type->toDot(out);

    int index = 0;
    for (auto element : elements) {
        out << MakeConnection(nodeId, element->nodeId, std::to_string(index++));
        element->toDot(out);
    }
}


void ElementCompositeLiteral::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Element");

    if (key != nullptr) {
        out << MakeConnection(nodeId, key->nodeId, "Key");
        key->toDot(out);
    }

    if (std::holds_alternative<ExpressionAST *>(value)) {
        auto temp = std::get<ExpressionAST *>(value);
        out << MakeConnection(nodeId, temp->nodeId);
        temp->toDot(out);
    } else if (std::holds_alternative<ElementCompositeLiteralList>(value)) {
        int index = 0;
        for (auto insideElement : std::get<ElementCompositeLiteralList>(value)) {
            out << MakeConnection(nodeId, insideElement->nodeId, std::to_string(index++));
            insideElement->toDot(out);
        }
    }
}

void AccessExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    out << MakeConnection(nodeId, base->nodeId, "base");
    base->toDot(out);

    out << MakeConnection(nodeId, accessor->nodeId, "access");
    accessor->toDot(out);
}


void CallableExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    out << MakeConnection(nodeId, base->nodeId, "base");
    base->toDot(out);

    int index = 0;
    for (auto arg : arguments) {
        out << MakeConnection(nodeId, arg->nodeId, std::to_string(index++));
        arg->toDot(out);
    }
}

void BinaryExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());
    out << MakeConnection(nodeId, lhs->nodeId, "left");
    out << MakeConnection(nodeId, rhs->nodeId, "right");
    lhs->toDot(out);
    rhs->toDot(out);
}

void UnaryExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());
    out << MakeConnection(nodeId, expression->nodeId);
    expression->toDot(out);
}

void FunctionLitExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    out << MakeConnection(nodeId, signature->nodeId);
    signature->toDot(out);

    out << MakeConnection(nodeId, block->nodeId);
    block->toDot(out);
}

void NilExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Nil");
}

void RuneExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Rune: " + std::to_string(runeLit));
}

void StringExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "String: " + stringLit);
}

void FloatExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Float: " + std::to_string(floatLit));
}

void BooleanExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Bool: " + std::to_string(boolLit));
}

void IntegerExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Int: " + std::to_string(intLit));
}

void IdentifierAsExpression::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, "Id: " + identifier);
}

void MethodDeclaration::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name() + " " + identifier);

    out << MakeConnection(nodeId, receiverType->nodeId, "receiver");
    receiverType->toDot(out);

    out << MakeConnection(nodeId, signature->nodeId);
    signature->toDot(out);

    out << MakeConnection(nodeId, block->nodeId);
    block->toDot(out);
}

void FunctionDeclaration::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name() + " " + identifier);

    out << MakeConnection(nodeId, signature->nodeId);
    signature->toDot(out);

    if (block != nullptr ) {
        out << MakeConnection(nodeId, block->nodeId);
        block->toDot(out);
    }
}

void TypeDeclaration::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name() + ": " + alias);
    out << MakeConnection(nodeId, declType->nodeId);
    declType->toDot(out);
}

void VariableDeclaration::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, (isConst ? "Const " : "") + name());

    int index = 0;
    for (const auto & value : values){
        out << MakeConnection(nodeId, value->nodeId, std::to_string(index++));
        value->toDot(out);
    }

    out << MakeConnection(nodeId, identifiersWithType->nodeId, "ids");
    identifiersWithType->toDot(out);
}

void IdentifierAsType::toDot(std::ostream &out) const noexcept {
    std::string label = "Type: " + identifier;

    if (isVariadic)
        label += "\n Variadic";

    if (isPointer)
        label += "\n Pointer";

    out << MakeNode(nodeId, label);
}

void InterfaceType::toDot(std::ostream &out) const noexcept {
    out << MakeNode(nodeId, name());

    for (auto func : functions) {
        out << MakeConnection(nodeId, func->nodeId);
        func->toDot(out);
    }
}
