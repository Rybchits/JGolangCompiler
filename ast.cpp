#include "ast.h"

std::string UnaryExpression::name() const noexcept {
    switch (this->type) {
        case UnaryNot:
            return "UnaryNot";
        case UnaryPlus:
            return "UnaryPlus";
        case UnaryMinus:
            return "UnaryMinus";
        case Increment:
            return "Increment";
        case Decrement:
            return "Decrement";
        case Variadic:
            return "Variadic";
    }
    return "UnaryExpr";
}

std::string BinaryExpression::name() const noexcept {
    switch (type) {
        case Addition:
            return "Addition";
        case Subtraction:
            return "Subtraction";
        case Multiplication:
            return "Multiplication";
        case Division:
            return "Division";
        case Mod:
            return "Mod";
        case And:
            return "And";
        case Or:
            return "Or";
        case Equal:
            return "Equal";
        case Greater:
            return "Greater";
        case Less:
            return "Less";
        case NotEqual:
            return "NotEqual";
        case LessOrEqual:
            return "LessOrEqual";
        case GreatOrEqual:
            return "GreatOrEqual";
    }
    return "BinaryExpr";
}

std::string KeywordStatement::name() const noexcept {
    switch (type) {
        case Break:
            return "BreakStmt";
        case Continue:
            return "ContinueStmt";
        case Fallthrough:
            return "FallthroughStmt";
    }
    return "KeywordStmt";
}

std::string AssignmentStatement::name() const noexcept {
    switch (type) {
        case SimpleAssign:
            return "Op =";
        case MinusAssign:
            return "Op -=";
        case PlusAssign:
            return "Op +=";
        case ModAssign:
            return "Op %=";
        case MulAssign:
            return "Op *=";
        case DivAssign:
            return "Op /=";
    }
    return "AssignmentStmt";
}


std::string AccessExpression::name() const noexcept {
    switch (type) {
        case Indexing:
            return "Indexing";
        case FieldSelect:
            return "FieldSelect";
    }
    return "AccessExpr";
}

bool BinaryExpression::isLogical() {
    return type == Or || type == And;
}

bool BinaryExpression::isComparison() {
    return type == Equal || type == Greater || type == Less || type == NotEqual || type == LessOrEqual || type == GreatOrEqual;
}

AssignmentStatement::AssignmentStatement(AssignmentEnum type, ExpressionASTPtr left, ExpressionASTPtr right)
    : AssignmentStatement(type, MakeList<ExpressionList>(std::move(left)),
                          MakeList<ExpressionList>(std::move(right))) {}

AssignmentStatement::AssignmentStatement(AssignmentEnum type, ExpressionList left, ExpressionList right)
    : type(type), rhs(std::move(right)) {
    for (auto& expression : left) {
        if (auto* access = dynamic_cast<AccessExpression*>(expression.get());
            access && access->type == AccessExpression::Indexing) {
            indexes.push_back(std::move(access->accessor));
            lhs.push_back(std::move(access->base));
        } else {
            if (auto* identifier = dynamic_cast<IdentifierAsExpression*>(expression.get())) {
                identifier->isDestination = true;
            }
            indexes.push_back(nullptr);
            lhs.push_back(std::move(expression));
        }
    }
}

void PackageAST::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& decl : topDeclarations) {
        decl->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void VariableDeclaration::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    identifiersWithType->acceptVisitor(visitor);

    for (const auto& expr : values ) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void TypeDeclaration::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    declType->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void FunctionDeclaration::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (block != nullptr) {
        block->acceptVisitor(visitor);
    }

    signature->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void MethodDeclaration::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    signature->acceptVisitor(visitor);

    if (receiverType != nullptr) {
        receiverType->acceptVisitor(visitor);
    }

    if (block != nullptr) {
        block->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void IdentifierAsExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void IntegerExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void BooleanExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void FloatExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void StringExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void NilExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void FunctionLitExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    signature->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void UnaryExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    expression->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void BinaryExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    lhs->acceptVisitor(visitor);
    rhs->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void CallableExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    base->acceptVisitor(visitor);

    for (const auto& arg : arguments ) {
        arg->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void AccessExpression::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    base->acceptVisitor(visitor);
    accessor->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void ElementCompositeLiteral::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (key != nullptr) {
        key->acceptVisitor(visitor);
    }

    if (std::holds_alternative<ExpressionASTPtr>(value)) {
        std::get<ExpressionASTPtr>(value)->acceptVisitor(visitor);

    } else if (std::holds_alternative<ElementCompositeLiteralList>(value)) {
        for (const auto& el : std::get<ElementCompositeLiteralList>(value)) {
            el->acceptVisitor(visitor);
        }
    }

    visitor->onFinishVisit(*this);
}

void CompositeLiteral::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (type != nullptr) {
        type->acceptVisitor(visitor);
    }

    for (const auto& el : elements) {
        el->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void BlockStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& stmt : body) {
        stmt->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void KeywordStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void ExpressionStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    expression->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void AssignmentStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& expr : lhs) {
        expr->acceptVisitor(visitor);
    }

    for (const auto& expr : rhs) {
        expr->acceptVisitor(visitor);
    }

    for (const auto& expr : indexes) {
        if (expr != nullptr) {
            expr->acceptVisitor(visitor);
        }
    }

    visitor->onFinishVisit(*this);
}

void ForStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (initStatement != nullptr) {
        initStatement->acceptVisitor(visitor);
    }

    if (conditionExpression != nullptr) {
        conditionExpression->acceptVisitor(visitor);
    }

    if (iterationStatement != nullptr) {
        iterationStatement->acceptVisitor(visitor);
    }

    block->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void WhileStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    conditionExpression->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void ForRangeStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    expressionValue->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    for (const auto& expr : initStatement) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void ReturnStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& expr : returnValues) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void IfStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    condition->acceptVisitor(visitor);
    thenStatement->acceptVisitor(visitor);

    if (preStatement != nullptr) {
        preStatement->acceptVisitor(visitor);
    }

    if (elseStatement) {
        elseStatement->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void SwitchCaseClause::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (expressionCase != nullptr) {
        expressionCase->acceptVisitor(visitor);
    }

    block->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void SwitchStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (statement != nullptr) {
        statement->acceptVisitor(visitor);
    }

    expression->acceptVisitor(visitor);

    for (const auto& caseClause : clauseList) {
        caseClause->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void DeclarationStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& decl : declarations) {
        decl->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void IdentifiersWithType::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    if (type != nullptr) {
        type->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void FunctionSignature::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& arg : idsAndTypesArgs) {
        arg->acceptVisitor(visitor);
    }

    for (const auto& result : idsAndTypesResults) {
        result->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void ArraySignature::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    arrayElementType->acceptVisitor(visitor);

    visitor->onFinishVisit(*this);
}

void StructSignature::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& member : structMembers) {
        member->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void IdentifierAsType::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);
    visitor->onFinishVisit(*this);
}

void InterfaceType::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& fn : functions) {
        fn->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}

void ShortVarDeclarationStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& expr : values) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}
