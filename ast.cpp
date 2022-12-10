#include "ast.h"

/* -------------------------------- Types -------------------------------- */
std::list<std::string> BuiltInTypes = {
        "int",
        "int8",
        "int16",
        "int32",
        "int64",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "float32",
        "float64",
        "string",
        "rune",
        "bool"
};

template <typename T>
T* As(NodeAST* node) {
    return dynamic_cast<T*>(node);
}

[[nodiscard]] bool IdentifierAsType::isBuiltInType() const {
    return std::find(BuiltInTypes.begin(), BuiltInTypes.end(), identifier) != BuiltInTypes.end();
}


TypeList* ListIdentifiersToListTypes(IdentifiersList& identifiers) {
    auto listTypes = new std::list<TypeAST *>;

    if (identifiers.empty()) { return listTypes; }

    for (const auto& id: identifiers) {
        listTypes->push_back(new IdentifierAsType(id));
    }

    return listTypes;
}


std::list<IdentifiersWithType *> *AttachIdentifiersToListTypes(TypeList& listTypes) {
    auto listIdentifiersWithType = new std::list<IdentifiersWithType *>;

    if (listTypes.empty()) { return listIdentifiersWithType; }

    for (auto type: listTypes) {
        auto ids = new IdentifiersList();
        ids->push_back("_");
        listIdentifiersWithType->push_back(new IdentifiersWithType(*ids, type));
    }

    return listIdentifiersWithType;
}


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
        case ShortDeclaration:
            return "Op :=";
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

/* -------------------------------- Visitors -------------------------------- */
void PackageAST::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    for (auto* decl : *topDeclarations) {
        decl->acceptVisitor(visitor);
    }
}

void VariableDeclaration::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    identifiersWithType->acceptVisitor(visitor);

    for (auto* expr : values ) {
        expr->acceptVisitor(visitor);
    }
}

void TypeDeclaration::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    declType->acceptVisitor(visitor);
}

void FunctionDeclaration::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    if (block != nullptr) {
        block->acceptVisitor(visitor);
    }

    signature->acceptVisitor(visitor);
}

void MethodDeclaration::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    signature->acceptVisitor(visitor);

    if (receiverType != nullptr) {
        receiverType->acceptVisitor(visitor);
    }

    if (block != nullptr) {
        block->acceptVisitor(visitor);
    }
}

void IdentifierAsExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void IntegerExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void BooleanExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void FloatExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void StringExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void RuneExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void NilExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void FunctionLitExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    signature->acceptVisitor(visitor);
    block->acceptVisitor(visitor);
}

void UnaryExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    expression->acceptVisitor(visitor);
}

void BinaryExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    lhs->acceptVisitor(visitor);
    rhs->acceptVisitor(visitor);
}

void CallableExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    base->acceptVisitor(visitor);

    for (auto* arg : arguments ) {
        arg->acceptVisitor(visitor);
    }
}

void AccessExpression::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    base->acceptVisitor(visitor);
    accessor->acceptVisitor(visitor);
}

void ElementCompositeLiteral::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    if (key != nullptr) {
        key->acceptVisitor(visitor);
    }

    if (std::holds_alternative<ExpressionAST *>(value)) {
        std::get<ExpressionAST *>(value)->acceptVisitor(visitor);

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(value)) {
        for (auto el : std::get<std::list<ElementCompositeLiteral *>>(value)) {
            el->acceptVisitor(visitor);
        }
    }
}

void CompositeLiteral::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    if (type != nullptr) {
        type->acceptVisitor(visitor);
    }

    for (auto el : elements) {
        el->acceptVisitor(visitor);
    }
}

void BlockStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    for (auto stmt : body) {
        stmt->acceptVisitor(visitor);
    }
}

void KeywordStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void ExpressionStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    expression->acceptVisitor(visitor);
}

void AssignmentStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    for (auto expr : lhs) {
        expr->acceptVisitor(visitor);
    }

    for (auto expr : rhs) {
        expr->acceptVisitor(visitor);
    }
}

void ForStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

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
}

void WhileStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    conditionExpression->acceptVisitor(visitor);
    block->acceptVisitor(visitor);
}

void ForRangeStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    expressionValue->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    for (auto expr : initStatement) {
        expr->acceptVisitor(visitor);
    }
}

void ReturnStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    for (auto expr : returnValues) {
        expr->acceptVisitor(visitor);
    }
}

void IfStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    condition->acceptVisitor(visitor);
    thenStatement->acceptVisitor(visitor);

    if (preStatement != nullptr) {
        preStatement->acceptVisitor(visitor);
    }

    if (elseStatement) {
        elseStatement->acceptVisitor(visitor);
    }
}

void SwitchCaseClause::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    expressionCase->acceptVisitor(visitor);
    for (auto statement : statementsList) {
        statement->acceptVisitor(visitor);
    }
}

void SwitchStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    if (statement != nullptr) {
        statement->acceptVisitor(visitor);
    }

    expression->acceptVisitor(visitor);

    for (auto caseClause : clauseList) {
        caseClause->acceptVisitor(visitor);
    }

    for (auto defaultStmt : defaultStatement) {
        defaultStmt->acceptVisitor(visitor);
    }
}

void DeclarationStatement::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    for (auto decl : declarations) {
        decl->acceptVisitor(visitor);
    }
}

void IdentifiersWithType::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    if (type != nullptr) {
        type->acceptVisitor(visitor);
    }
}

void FunctionSignature::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    for (auto arg : idsAndTypesArgs) {
        arg->acceptVisitor(visitor);
    }

    for (auto result : idsAndTypesResults) {
        result->acceptVisitor(visitor);
    }
}

void ArraySignature::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
    arrayElementType->acceptVisitor(visitor);
}

void StructSignature::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);

    for (auto member : structMembers) {
        member->acceptVisitor(visitor);
    }
}

void IdentifierAsType::acceptVisitor(Visitor* visitor) const noexcept {
    visitor->visit(this);
}

void InterfaceType::acceptVisitor(Visitor *visitor) const noexcept {
    visitor->visit(this);

    for (auto fn : this->functions) {
        fn->acceptVisitor(visitor);
    }
}