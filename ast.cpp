#include "ast.h"

TypeList ListIdentifiersToListTypes(const IdentifiersList& identifiers) {
    TypeList types;
    for (const auto& id : identifiers) {
        types.push_back(std::make_unique<IdentifierAsType>(id));
    }
    return types;
}

IdentifiersWithTypeList AttachIdentifiersToListTypes(TypeList listTypes) {
    IdentifiersWithTypeList result;
    for (auto& type : listTypes) {
        result.push_back(std::make_unique<IdentifiersWithType>("_", std::move(type)));
    }
    return result;
}

std::optional<IdentifiersList> IdentifiersListFromExpressions(const ExpressionList& expressions) {
    IdentifiersList identifiers;
    for (const auto& expression : expressions) {
        auto* identifier = dynamic_cast<IdentifierAsExpression*>(expression.get());
        if (!identifier) return std::nullopt;
        identifiers.push_back(identifier->identifier);
    }
    return identifiers;
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

/* -------------------------------- Visitors -------------------------------- */
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

    for (const auto& fn : this->functions) {
        fn->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
}


/* -------------------------------- Clone -------------------------------- */

NodeASTPtr PackageAST::cloneImpl() const {
    DeclarationList cloneDeclList;

    for (const auto& decl : topDeclarations) {
        cloneDeclList.push_back(decl->clone());
    }

    return std::make_unique<PackageAST>(packageName, std::move(cloneDeclList));
}

NodeASTPtr VariableDeclaration::cloneImpl() const {
    ExpressionList cloneValues;
    
    for (const auto& value : this->values) {
        cloneValues.push_back(value->clone());
    }

    IdentifiersWithTypePtr cloneTypedIds = this->identifiersWithType? this->identifiersWithType->clone() : nullptr;
    return std::make_unique<VariableDeclaration>(std::move(cloneTypedIds), std::move(cloneValues), this->isConst);
}

NodeASTPtr TypeDeclaration::cloneImpl() const {
    TypeASTPtr cloneType = this->declType? this->declType->clone() : nullptr;
    return std::make_unique<TypeDeclaration>(this->alias, std::move(cloneType));
}

NodeASTPtr FunctionDeclaration::cloneImpl() const {
    FunctionSignaturePtr cloneSignature = this->signature? this->signature->clone() : nullptr;
    BlockStatementPtr cloneBlock = this->block? this->block->clone() : nullptr;

    return std::make_unique<FunctionDeclaration>(this->identifier, std::move(cloneSignature), std::move(cloneBlock));
}

NodeASTPtr MethodDeclaration::cloneImpl() const {
    FunctionSignaturePtr cloneSignature = this->signature? this->signature->clone() : nullptr;
    BlockStatementPtr cloneBlock = this->block? this->block->clone() : nullptr;
    IdentifierAsTypePtr cloneReceiverType = this->receiverType? this->receiverType->clone() : nullptr;

    return std::make_unique<MethodDeclaration>(this->identifier, this->receiverIdentifier, std::move(cloneReceiverType), std::move(cloneSignature), std::move(cloneBlock));
}

NodeASTPtr IdentifierAsExpression::cloneImpl() const {
    auto copy = std::make_unique<IdentifierAsExpression>(this->identifier);
    copy->isDestination = isDestination;
    return copy;
}

NodeASTPtr IntegerExpression::cloneImpl() const {
    return std::make_unique<IntegerExpression>(this->intLit);
}

NodeASTPtr BooleanExpression::cloneImpl() const {
    return std::make_unique<BooleanExpression>(this->boolLit);
}

NodeASTPtr FloatExpression::cloneImpl() const {
    return std::make_unique<FloatExpression>(this->floatLit);
}

NodeASTPtr StringExpression::cloneImpl() const {
    return std::make_unique<StringExpression>(this->stringLit);
}

NodeASTPtr NilExpression::cloneImpl() const {
    return std::make_unique<NilExpression>();
}

NodeASTPtr FunctionLitExpression::cloneImpl() const {
    FunctionSignaturePtr cloneSignature = this->signature? this->signature->clone() : nullptr;
    BlockStatementPtr cloneBlock = this->block? this->block->clone() : nullptr;

    return std::make_unique<FunctionLitExpression>(std::move(cloneSignature), std::move(cloneBlock));
}

NodeASTPtr UnaryExpression::cloneImpl() const {
    ExpressionASTPtr cloneExpression = this->expression? this->expression->clone() : nullptr;
    return std::make_unique<UnaryExpression>(this->type, std::move(cloneExpression));
}

NodeASTPtr BinaryExpression::cloneImpl() const {
    ExpressionASTPtr cloneLhs = this->lhs? this->lhs->clone() : nullptr;
    ExpressionASTPtr cloneRhs = this->rhs? this->rhs->clone() : nullptr;

    return std::make_unique<BinaryExpression>(this->type, std::move(cloneLhs), std::move(cloneRhs));
}

NodeASTPtr CallableExpression::cloneImpl() const {
    ExpressionList cloneArguments;
    
    for (const auto& arg : this->arguments) {
        cloneArguments.push_back(arg->clone());
    }
    
    ExpressionASTPtr cloneBase = this->base? this->base->clone() : nullptr;

    return std::make_unique<CallableExpression>(std::move(cloneBase), std::move(cloneArguments));
}

NodeASTPtr AccessExpression::cloneImpl() const {
    ExpressionASTPtr cloneBase = this->base? this->base->clone() : nullptr;
    ExpressionASTPtr cloneAccessor = this->accessor? this->accessor->clone() : nullptr;

    return std::make_unique<AccessExpression>(this->type, std::move(cloneBase), std::move(cloneAccessor));
}

NodeASTPtr ElementCompositeLiteral::cloneImpl() const {
    ExpressionASTPtr cloneKey = this->key? this->key->clone() : nullptr;

    if (std::holds_alternative<ExpressionASTPtr>(value)) {
        return std::make_unique<ElementCompositeLiteral>(std::move(cloneKey), std::get<ExpressionASTPtr>(this->value)->clone());

    } else if (std::holds_alternative<ElementCompositeLiteralList>(value)) {
        ElementCompositeLiteralList cloneElements;

        for (const auto& el : std::get<ElementCompositeLiteralList>(value)) {
            cloneElements.push_back(el->clone());
        }


        return std::make_unique<ElementCompositeLiteral>(std::move(cloneKey), std::move(cloneElements));
    }

    return nullptr;
}

NodeASTPtr CompositeLiteral::cloneImpl() const {
    ElementCompositeLiteralList cloneElements;

    for (const auto& decl : this->elements) {
        cloneElements.push_back(decl->clone());
    }

    TypeASTPtr cloneType = this->type? this->type->clone() : nullptr;

    return std::make_unique<CompositeLiteral>(std::move(cloneType), std::move(cloneElements));
}


NodeASTPtr BlockStatement::cloneImpl() const {
    StatementList cloneStatements;

    for (const auto& stmt : this->body) {
        cloneStatements.push_back(stmt->clone());
    }

    return std::make_unique<BlockStatement>(std::move(cloneStatements));
}


NodeASTPtr KeywordStatement::cloneImpl() const {
    return std::make_unique<KeywordStatement>(this->type);
}

NodeASTPtr ExpressionStatement::cloneImpl() const {
    ExpressionASTPtr cloneExpression = this->expression? this->expression->clone() : nullptr;
    return std::make_unique<ExpressionStatement>(std::move(cloneExpression));
}

NodeASTPtr AssignmentStatement::cloneImpl() const {
    ExpressionList cloneLhs;

    for (const auto& expr : this->lhs) {
        cloneLhs.push_back(expr->clone());
    }

    ExpressionList cloneRhs;

    for (const auto& expr : this->rhs) {
        cloneRhs.push_back(expr->clone());
    }

    auto copy = std::make_unique<AssignmentStatement>(this->type, ExpressionList{}, ExpressionList{});
    copy->lhs = std::move(cloneLhs);
    copy->rhs = std::move(cloneRhs);
    for (const auto& index : indexes) {
        copy->indexes.push_back(index ? index->clone() : nullptr);
    }
    return copy;
}

NodeASTPtr ForStatement::cloneImpl() const {
    StatementASTPtr cloneInitStatement = this->initStatement? this->initStatement->clone() : nullptr;
    ExpressionASTPtr cloneCondition = this->conditionExpression? this->conditionExpression->clone() : nullptr;
    BlockStatementPtr cloneBlock = this->block? this->block->clone() : nullptr;
    StatementASTPtr cloneIterationStatement = this->iterationStatement? this->iterationStatement->clone() : nullptr;

    return std::make_unique<ForStatement>(std::move(cloneInitStatement), std::move(cloneCondition), std::move(cloneIterationStatement), std::move(cloneBlock));
}

NodeASTPtr WhileStatement::cloneImpl() const {
    ExpressionASTPtr cloneConditionExpression = this->conditionExpression? this->conditionExpression->clone() : nullptr;
    BlockStatementPtr cloneBlock = this->block? this->block->clone() : nullptr;
    return std::make_unique<WhileStatement>(std::move(cloneConditionExpression), std::move(cloneBlock));
}

NodeASTPtr ForRangeStatement::cloneImpl() const {
    ExpressionList cloneInits;

    for (const auto& expr : this->initStatement) {
        cloneInits.push_back(expr->clone());
    }

    ExpressionASTPtr cloneExpressionValue = this->expressionValue? this->expressionValue->clone() : nullptr;
    BlockStatementPtr cloneBlock = this->block? this->block->clone() : nullptr;

    return std::make_unique<ForRangeStatement>(std::move(cloneInits), std::move(cloneExpressionValue), std::move(cloneBlock), this->hasShortDeclaration);
}

NodeASTPtr ReturnStatement::cloneImpl() const {
    ExpressionList cloneReturns;

    for (const auto& expr : this->returnValues) {
        cloneReturns.push_back(expr->clone());
    }

    return std::make_unique<ReturnStatement>(std::move(cloneReturns));
}

NodeASTPtr IfStatement::cloneImpl() const {
    StatementASTPtr clonePreStatement = this->preStatement? this->preStatement->clone() : nullptr;
    ExpressionASTPtr cloneCondition = this->condition? this->condition->clone() : nullptr;
    BlockStatementPtr cloneThenStatement = this->thenStatement? this->thenStatement->clone() : nullptr;
    StatementASTPtr cloneElseStatement = this->elseStatement? this->elseStatement->clone() : nullptr;

    return std::make_unique<IfStatement>(std::move(clonePreStatement), std::move(cloneCondition), std::move(cloneThenStatement), std::move(cloneElseStatement));
}


NodeASTPtr SwitchCaseClause::cloneImpl() const {
    StatementList cloneStatements;

    for (const auto& stms : this->block->body) {
        cloneStatements.push_back(stms->clone());
    }

    ExpressionASTPtr cloneExpressionCase = this->expressionCase? this->expressionCase->clone() : nullptr;

    auto copy = std::make_unique<SwitchCaseClause>(std::move(cloneExpressionCase), std::make_unique<BlockStatement>(std::move(cloneStatements)));
    copy->fallthrowEnds = fallthrowEnds;
    return copy;
}


NodeASTPtr SwitchStatement::cloneImpl() const {
    SwitchCaseList cloneClauseList;

    for (const auto& clause : this->clauseList) {
        cloneClauseList.push_back(clause->clone());
    }

    StatementASTPtr cloneStatement = this->statement? this->statement->clone() : nullptr;
    ExpressionASTPtr cloneExpression = this->expression? this->expression->clone() : nullptr;

    return std::make_unique<SwitchStatement>(std::move(cloneStatement), std::move(cloneExpression), std::move(cloneClauseList));
}

NodeASTPtr DeclarationStatement::cloneImpl() const {
    DeclarationList cloneDecls;

    for (const auto& stms : this->declarations) {
        cloneDecls.push_back(stms->clone());
    }

    return std::make_unique<DeclarationStatement>(std::move(cloneDecls));
}

NodeASTPtr IdentifiersWithType::cloneImpl() const {
    IdentifiersList cloneIdentifiers;

    for (const auto& id : this->identifiers) {
        cloneIdentifiers.push_back(id);
    }
    TypeASTPtr cloneType = this->type? this->type->clone() : nullptr;

    return std::make_unique<IdentifiersWithType>(std::move(cloneIdentifiers), std::move(cloneType));
}


NodeASTPtr FunctionSignature::cloneImpl() const {
    IdentifiersWithTypeList cloneIdsAndTypesArgs;
    IdentifiersWithTypeList cloneIdsAndTypesResults;

    for (const auto& id : this->idsAndTypesArgs) {
        cloneIdsAndTypesArgs.push_back(id->clone());
    }

    for (const auto& id : this->idsAndTypesResults) {
        cloneIdsAndTypesResults.push_back(id->clone());
    }

    return std::make_unique<FunctionSignature>(std::move(cloneIdsAndTypesArgs), std::move(cloneIdsAndTypesResults));
}

NodeASTPtr ArraySignature::cloneImpl() const {
    TypeASTPtr cloneArrayElementType = this->arrayElementType? this->arrayElementType->clone() : nullptr;
    return std::make_unique<ArraySignature>(std::move(cloneArrayElementType), this->dimensions);
}

NodeASTPtr IdentifierAsType::cloneImpl() const {
    return std::make_unique<IdentifierAsType>(this->identifier);
}

NodeASTPtr StructSignature::cloneImpl() const {
    IdentifiersWithTypeList cloneMembers;

    for (const auto& typedId : this->structMembers) {
        cloneMembers.push_back(typedId->clone());
    }

    return std::make_unique<StructSignature>(std::move(cloneMembers));
}

NodeASTPtr InterfaceType::cloneImpl() const {
    FunctionList cloneFunctions;

    for (const auto& function : this->functions) {
        cloneFunctions.push_back(function->clone());
    }

    return std::make_unique<InterfaceType>(std::move(cloneFunctions));
}


void ShortVarDeclarationStatement::acceptVisitor(Visitor* visitor) {
    visitor->onStartVisit(*this);

    for (const auto& expr : this->values) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(*this);
};

NodeASTPtr ShortVarDeclarationStatement::cloneImpl() const {
    ExpressionList cloneValues;

    for (const auto& expr : this->values) {
        cloneValues.push_back(expr->clone());
    }

    IdentifiersList cloneIdentifiers;

    for (const auto& id : this->identifiers) {
        cloneIdentifiers.push_back(id);
    }

    return std::make_unique<ShortVarDeclarationStatement>(std::move(cloneIdentifiers), std::move(cloneValues));
};