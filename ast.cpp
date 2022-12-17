#include "ast.h"

/* -------------------------------- Types -------------------------------- */
std::list<std::string> BuiltInTypes = {
        "int",
        "int8",
        "int16",
        "int32",
        "int64",
        "float32",
        "float64",
        "string",
        "rune",
        "bool"
};


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

// If there is an indexing, we decompose the left part of the assignment
AssignmentStatement::AssignmentStatement(AssignmentEnum type, ExpressionAST* lExp, ExpressionAST* rExp) {
    this->type = type;
    
    AccessExpression* leftIndexingElement = dynamic_cast<AccessExpression*>(lExp);

    if (leftIndexingElement != nullptr && leftIndexingElement->type == AccessExpressionEnum::Indexing) {
        indexes.push_back(leftIndexingElement->accessor);
        lhs.push_back(leftIndexingElement->base);
    } else {
        indexes.push_back(nullptr);
        lhs.push_back(lExp);
    }
    
    rhs.push_back(rExp);
}


AssignmentStatement::AssignmentStatement(AssignmentEnum type, ExpressionList& lExpList, ExpressionList& rExpList) {
    this->type = type;

    for (auto left : lExpList) {
        AccessExpression* leftIndexingElement = dynamic_cast<AccessExpression*>(left);

        if (leftIndexingElement != nullptr && leftIndexingElement->type == AccessExpressionEnum::Indexing) {
            indexes.push_back(leftIndexingElement->accessor);
            lhs.push_back(leftIndexingElement->base);
        } else {
            indexes.push_back(nullptr);
            lhs.push_back(left);
        }
    }

    rhs = rExpList;
}


/* -------------------------------- Visitors -------------------------------- */
void PackageAST::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto* decl : *topDeclarations) {
        decl->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void VariableDeclaration::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    identifiersWithType->acceptVisitor(visitor, way);

    for (auto* expr : values ) {
        expr->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void TypeDeclaration::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    declType->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void FunctionDeclaration::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    if (block != nullptr) {
        block->acceptVisitor(visitor, way);
    }

    signature->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void MethodDeclaration::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    signature->acceptVisitor(visitor, way);

    if (receiverType != nullptr) {
        receiverType->acceptVisitor(visitor, way);
    }

    if (block != nullptr) {
        block->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void IdentifierAsExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void IntegerExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void BooleanExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void FloatExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void StringExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void RuneExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void NilExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void FunctionLitExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    signature->acceptVisitor(visitor, way);
    block->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void UnaryExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    expression->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void BinaryExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    lhs->acceptVisitor(visitor, way);
    rhs->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void CallableExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    base->acceptVisitor(visitor, way);

    for (auto* arg : arguments ) {
        arg->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void AccessExpression::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    base->acceptVisitor(visitor, way);
    accessor->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void ElementCompositeLiteral::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    if (key != nullptr) {
        key->acceptVisitor(visitor, way);
    }

    if (std::holds_alternative<ExpressionAST *>(value)) {
        std::get<ExpressionAST *>(value)->acceptVisitor(visitor, way);

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(value)) {
        for (auto el : std::get<std::list<ElementCompositeLiteral *>>(value)) {
            el->acceptVisitor(visitor, way);
        }
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void CompositeLiteral::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    if (type != nullptr) {
        type->acceptVisitor(visitor, way);
    }

    for (auto el : elements) {
        el->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void BlockStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto stmt : body) {
        stmt->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void KeywordStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void ExpressionStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    expression->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void AssignmentStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto expr : lhs) {
        expr->acceptVisitor(visitor, way);
    }

    for (auto expr : rhs) {
        expr->acceptVisitor(visitor, way);
    }

    for (auto expr : indexes) {
        if (expr != nullptr) {
            expr->acceptVisitor(visitor, way);
        }
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}


void ForStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    if (initStatement != nullptr) {
        initStatement->acceptVisitor(visitor, way);
    }

    if (conditionExpression != nullptr) {
        conditionExpression->acceptVisitor(visitor, way);
    }

    if (iterationStatement != nullptr) {
        iterationStatement->acceptVisitor(visitor, way);
    }

    block->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void WhileStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    conditionExpression->acceptVisitor(visitor, way);
    block->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void ForRangeStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    expressionValue->acceptVisitor(visitor, way);
    block->acceptVisitor(visitor, way);

    for (auto expr : initStatement) {
        expr->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void ReturnStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto expr : returnValues) {
        expr->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void IfStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    condition->acceptVisitor(visitor, way);
    thenStatement->acceptVisitor(visitor, way);

    if (preStatement != nullptr) {
        preStatement->acceptVisitor(visitor, way);
    }

    if (elseStatement) {
        elseStatement->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void SwitchCaseClause::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    expressionCase->acceptVisitor(visitor, way);
    for (auto statement : statementsList) {
        statement->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void SwitchStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    if (statement != nullptr) {
        statement->acceptVisitor(visitor, way);
    }

    expression->acceptVisitor(visitor, way);

    for (auto caseClause : clauseList) {
        caseClause->acceptVisitor(visitor, way);
    }

    for (auto defaultStmt : defaultStatement) {
        defaultStmt->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void DeclarationStatement::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto decl : declarations) {
        decl->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void IdentifiersWithType::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    if (type != nullptr) {
        type->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void FunctionSignature::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto arg : idsAndTypesArgs) {
        arg->acceptVisitor(visitor, way);
    }

    for (auto result : idsAndTypesResults) {
        result->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void ArraySignature::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    arrayElementType->acceptVisitor(visitor, way);

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void StructSignature::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto member : structMembers) {
        member->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}

void IdentifierAsType::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    visitor->visit(this);
}

void InterfaceType::acceptVisitor(Visitor* visitor, TraversalMethod way) noexcept {
    if (way == TraversalMethod::Downward)
        visitor->visit(this);

    for (auto fn : this->functions) {
        fn->acceptVisitor(visitor, way);
    }

    if (way == TraversalMethod::Upward)
        visitor->visit(this);
}