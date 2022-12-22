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

IdentifiersList* IdentifiersListFromExpressions(ExpressionList& expressions) {
    auto identifiers = new IdentifiersList();

    for (auto expr : expressions) {
        if (auto expressionAsIdentifier = dynamic_cast<IdentifierAsExpression*>(expr)) {
            identifiers->push_back(expressionAsIdentifier->identifier);
        } else {
            return nullptr;
        }
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
void PackageAST::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto* decl : topDeclarations) {
        decl->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void VariableDeclaration::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    identifiersWithType->acceptVisitor(visitor);

    for (auto* expr : values ) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void TypeDeclaration::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    declType->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void FunctionDeclaration::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    if (block != nullptr) {
        block->acceptVisitor(visitor);
    }

    signature->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void MethodDeclaration::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    signature->acceptVisitor(visitor);

    if (receiverType != nullptr) {
        receiverType->acceptVisitor(visitor);
    }

    if (block != nullptr) {
        block->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void IdentifierAsExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void IntegerExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void BooleanExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void FloatExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void StringExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void RuneExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void NilExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void FunctionLitExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    signature->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void UnaryExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    expression->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void BinaryExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    lhs->acceptVisitor(visitor);
    rhs->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void CallableExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    base->acceptVisitor(visitor);

    for (auto* arg : arguments ) {
        arg->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void AccessExpression::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    base->acceptVisitor(visitor);
    accessor->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void ElementCompositeLiteral::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

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

    visitor->onFinishVisit(this);
}

void CompositeLiteral::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    if (type != nullptr) {
        type->acceptVisitor(visitor);
    }

    for (auto el : elements) {
        el->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void BlockStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto stmt : body) {
        stmt->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void KeywordStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void ExpressionStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    expression->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void AssignmentStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto expr : lhs) {
        expr->acceptVisitor(visitor);
    }

    for (auto expr : rhs) {
        expr->acceptVisitor(visitor);
    }

    for (auto expr : indexes) {
        if (expr != nullptr) {
            expr->acceptVisitor(visitor);
        }
    }

    visitor->onFinishVisit(this);
}


void ForStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

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

    visitor->onFinishVisit(this);
}

void WhileStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    conditionExpression->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void ForRangeStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    expressionValue->acceptVisitor(visitor);
    block->acceptVisitor(visitor);

    for (auto expr : initStatement) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void ReturnStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto expr : returnValues) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void IfStatement::acceptVisitor(Visitor* visitor) noexcept {
    
    visitor->onStartVisit(this);

    condition->acceptVisitor(visitor);
    thenStatement->acceptVisitor(visitor);

    if (preStatement != nullptr) {
        preStatement->acceptVisitor(visitor);
    }

    if (elseStatement) {
        elseStatement->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void SwitchCaseClause::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    expressionCase->acceptVisitor(visitor);
    for (auto statement : statementsList) {
        statement->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void SwitchStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

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

    visitor->onFinishVisit(this);
}

void DeclarationStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto decl : declarations) {
        decl->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void IdentifiersWithType::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    if (type != nullptr) {
        type->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void FunctionSignature::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto arg : idsAndTypesArgs) {
        arg->acceptVisitor(visitor);
    }

    for (auto result : idsAndTypesResults) {
        result->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void ArraySignature::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    arrayElementType->acceptVisitor(visitor);

    visitor->onFinishVisit(this);
}

void StructSignature::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto member : structMembers) {
        member->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}

void IdentifierAsType::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);
    visitor->onFinishVisit(this);
}

void InterfaceType::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto fn : this->functions) {
        fn->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
}


/* -------------------------------- Clone -------------------------------- */

PackageAST* PackageAST::clone() const noexcept {
    DeclarationList cloneDeclList;

    for (auto decl : topDeclarations) {
        cloneDeclList.push_back(decl->clone());
    }

    return new PackageAST(packageName, cloneDeclList);
}

VariableDeclaration* VariableDeclaration::clone() const noexcept {
    ExpressionList cloneValues;
    
    for (auto value : this->values) {
        cloneValues.push_back(value->clone());
    }

    IdentifiersWithType* cloneTypedIds = this->identifiersWithType? this->identifiersWithType->clone() : nullptr;
    return new VariableDeclaration(cloneTypedIds, cloneValues, this->isConst);
}

TypeDeclaration* TypeDeclaration::clone() const noexcept {
    TypeAST* cloneType = this->declType? this->declType->clone() : nullptr;
    return new TypeDeclaration(this->alias, cloneType);
}

FunctionDeclaration* FunctionDeclaration::clone() const noexcept {
    FunctionSignature* cloneSignature = this->signature? this->signature->clone() : nullptr;
    BlockStatement* cloneBlock = this->block? this->block->clone() : nullptr;

    return new FunctionDeclaration(this->identifier, cloneSignature, cloneBlock);
}

MethodDeclaration* MethodDeclaration::clone() const noexcept {
    FunctionSignature* cloneSignature = this->signature? this->signature->clone() : nullptr;
    BlockStatement* cloneBlock = this->block? this->block->clone() : nullptr;
    IdentifierAsType* cloneReceiverType = this->receiverType? this->receiverType->clone() : nullptr;

    return new MethodDeclaration(this->identifier, this->receiverIdentifier, cloneReceiverType, cloneSignature, cloneBlock);
}

IdentifierAsExpression* IdentifierAsExpression::clone() const noexcept {
    return new IdentifierAsExpression(this->identifier);
}

IntegerExpression* IntegerExpression::clone() const noexcept {
    return new IntegerExpression(this->intLit);
}

BooleanExpression* BooleanExpression::clone() const noexcept {
    return new BooleanExpression(this->boolLit);
}

FloatExpression* FloatExpression::clone() const noexcept {
    return new FloatExpression(this->floatLit);
}

StringExpression* StringExpression::clone() const noexcept {
    return new StringExpression(this->stringLit);
}

RuneExpression* RuneExpression::clone() const noexcept {
    return new RuneExpression(this->runeLit);
}

NilExpression* NilExpression::clone() const noexcept {
    return new NilExpression();
}

FunctionLitExpression* FunctionLitExpression::clone() const noexcept {
    FunctionSignature* cloneSignature = this->signature? this->signature->clone() : nullptr;
    BlockStatement* cloneBlock = this->block? this->block->clone() : nullptr;

    return new FunctionLitExpression(cloneSignature, cloneBlock);
}

UnaryExpression* UnaryExpression::clone() const noexcept {
    ExpressionAST* cloneExpression = this->expression? this->expression->clone() : nullptr;
    return new UnaryExpression(this->type, cloneExpression);
}

BinaryExpression* BinaryExpression::clone() const noexcept {
    ExpressionAST* cloneLhs = this->lhs? this->lhs->clone() : nullptr;
    ExpressionAST* cloneRhs = this->rhs? this->rhs->clone() : nullptr;

    return new BinaryExpression(this->type, cloneLhs, cloneRhs);
}

CallableExpression* CallableExpression::clone() const noexcept {
    ExpressionList cloneArguments;
    
    for (auto arg : this->arguments) {
        cloneArguments.push_back(arg->clone());
    }
    
    ExpressionAST* cloneBase = this->base? this->base->clone() : nullptr;

    return new CallableExpression(cloneBase, cloneArguments);
}

AccessExpression* AccessExpression::clone() const noexcept {
    ExpressionAST* cloneBase = this->base? this->base->clone() : nullptr;
    ExpressionAST* cloneAccessor = this->accessor? this->accessor->clone() : nullptr;

    return new AccessExpression(this->type, cloneBase, cloneAccessor);
}

ElementCompositeLiteral* ElementCompositeLiteral::clone() const noexcept {
    ExpressionAST* cloneKey = this->key? this->key->clone() : nullptr;

    if (std::holds_alternative<ExpressionAST *>(value)) {
        return new ElementCompositeLiteral(cloneKey, std::get<ExpressionAST *>(this->value)->clone());

    } else if (std::holds_alternative<std::list<ElementCompositeLiteral *>>(value)) {
        std::list<ElementCompositeLiteral *> cloneElements;

        for (auto el : std::get<std::list<ElementCompositeLiteral *>>(value)) {
            cloneElements.push_back(el->clone());
        }


        return new ElementCompositeLiteral(cloneKey, cloneElements);
    }

    return nullptr;
}

CompositeLiteral* CompositeLiteral::clone() const noexcept {
    ElementCompositeLiteralList cloneElements;

    for (auto decl : this->elements) {
        cloneElements.push_back(decl->clone());
    }

    TypeAST* cloneType = this->type? this->type->clone() : nullptr;

    return new CompositeLiteral(cloneType, cloneElements);
}


BlockStatement* BlockStatement::clone() const noexcept {
    StatementList cloneStatements;

    for (auto stmt : this->body) {
        cloneStatements.push_back(stmt->clone());
    }

    return new BlockStatement(cloneStatements);
}


KeywordStatement* KeywordStatement::clone() const noexcept {
    return new KeywordStatement(this->type);
}

ExpressionStatement* ExpressionStatement::clone() const noexcept {
    ExpressionAST* cloneExpression = this->expression? this->expression->clone() : nullptr;
    return new ExpressionStatement(cloneExpression);
}

AssignmentStatement* AssignmentStatement::clone() const noexcept {
    ExpressionList cloneLhs;

    for (auto expr : this->lhs) {
        cloneLhs.push_back(expr->clone());
    }

    ExpressionList cloneRhs;

    for (auto expr : this->rhs) {
        cloneRhs.push_back(expr->clone());
    }

    return new AssignmentStatement(this->type, cloneLhs, cloneRhs);
}

ForStatement* ForStatement::clone() const noexcept {
    StatementAST* cloneInitStatement = this->initStatement? this->initStatement->clone() : nullptr;
    ExpressionAST* cloneCondition = this->conditionExpression? this->conditionExpression->clone() : nullptr;
    BlockStatement* cloneBlock = this->block? this->block->clone() : nullptr;
    StatementAST* cloneIterationStatement = this->iterationStatement? this->iterationStatement->clone() : nullptr;

    return new ForStatement(cloneInitStatement, cloneCondition, cloneIterationStatement, cloneBlock);
}

WhileStatement* WhileStatement::clone() const noexcept {
    ExpressionAST* cloneConditionExpression = this->conditionExpression? this->conditionExpression->clone() : nullptr;
    BlockStatement* cloneBlock = this->block? this->block->clone() : nullptr;
    return new WhileStatement(cloneConditionExpression, cloneBlock);
}

ForRangeStatement* ForRangeStatement::clone() const noexcept {
    ExpressionList cloneInits;

    for (auto expr : this->initStatement) {
        cloneInits.push_back(expr->clone());
    }

    ExpressionAST* cloneExpressionValue = this->expressionValue? this->expressionValue->clone() : nullptr;
    BlockStatement* cloneBlock = this->block? this->block->clone() : nullptr;

    return new ForRangeStatement(cloneInits, cloneExpressionValue, cloneBlock, this->hasShortDeclaration);
}

ReturnStatement* ReturnStatement::clone() const noexcept {
    ExpressionList cloneReturns;

    for (auto expr : this->returnValues) {
        cloneReturns.push_back(expr->clone());
    }

    return new ReturnStatement(cloneReturns);
}

IfStatement* IfStatement::clone() const noexcept {
    StatementAST* clonePreStatement = this->preStatement? this->preStatement->clone() : nullptr;
    ExpressionAST* cloneCondition = this->condition? this->condition->clone() : nullptr;
    BlockStatement* cloneThenStatement = this->thenStatement? this->thenStatement->clone() : nullptr;
    StatementAST* cloneElseStatement = this->elseStatement? this->elseStatement->clone() : nullptr;

    return new IfStatement(clonePreStatement, cloneCondition, cloneThenStatement, cloneElseStatement);
}


SwitchCaseClause* SwitchCaseClause::clone() const noexcept {
    StatementList cloneStatements;

    for (auto stms : this->statementsList) {
        cloneStatements.push_back(stms->clone());
    }

    ExpressionAST* cloneExpressionCase = this->expressionCase? this->expressionCase->clone() : nullptr;

    return new SwitchCaseClause(cloneExpressionCase, cloneStatements);
}


SwitchStatement* SwitchStatement::clone() const noexcept {
    std::list<SwitchCaseClause *> cloneClauseList;
    StatementList cloneDefaultStatements;

    for (auto stms : this->defaultStatement) {
        cloneDefaultStatements.push_back(stms->clone());
    }

    for (auto clause : this->clauseList) {
        cloneClauseList.push_back(clause->clone());
    }

    StatementAST* cloneStatement = this->statement? this->statement->clone() : nullptr;
    ExpressionAST* cloneExpression = this->expression? this->expression->clone() : nullptr;

    return new SwitchStatement(cloneStatement, cloneExpression, cloneClauseList, cloneDefaultStatements);
}

DeclarationStatement* DeclarationStatement::clone() const noexcept {
    DeclarationList cloneDecls;

    for (auto stms : this->declarations) {
        cloneDecls.push_back(stms->clone());
    }

    return new DeclarationStatement(cloneDecls);
}

IdentifiersWithType* IdentifiersWithType::clone() const noexcept {
    IdentifiersList cloneIdentifiers;

    for (auto id : this->identifiers) {
        cloneIdentifiers.push_back(id);
    }
    TypeAST *cloneType = this->type? this->type->clone() : nullptr;

    return new IdentifiersWithType(cloneIdentifiers, cloneType);
}


FunctionSignature* FunctionSignature::clone() const noexcept {
    std::list<IdentifiersWithType *> cloneIdsAndTypesArgs;
    std::list<IdentifiersWithType *> cloneIdsAndTypesResults;

    for (auto id : this->idsAndTypesArgs) {
        cloneIdsAndTypesArgs.push_back(id->clone());
    }

    for (auto id : this->idsAndTypesResults) {
        cloneIdsAndTypesResults.push_back(id->clone());
    }

    return new FunctionSignature(cloneIdsAndTypesArgs, cloneIdsAndTypesResults);
}

ArraySignature* ArraySignature::clone() const noexcept {
    TypeAST *cloneArrayElementType = this->arrayElementType? this->arrayElementType->clone() : nullptr;
    return new ArraySignature(cloneArrayElementType, this->dimensions);
}

IdentifierAsType* IdentifierAsType::clone() const noexcept {
    return new IdentifierAsType(this->identifier);
}

StructSignature* StructSignature::clone() const noexcept {
    std::list<IdentifiersWithType *> cloneMembers;

    for (auto typedId : this->structMembers) {
        cloneMembers.push_back(typedId->clone());
    }

    return new StructSignature(cloneMembers);
}

InterfaceType* InterfaceType::clone() const noexcept {
    FunctionList cloneFunctions;

    for (auto function : this->functions) {
        cloneFunctions.push_back(function->clone());
    }

    return new InterfaceType(cloneFunctions);
}


void ShortVarDeclarationStatement::acceptVisitor(Visitor* visitor) noexcept {
    visitor->onStartVisit(this);

    for (auto expr : this->values) {
        expr->acceptVisitor(visitor);
    }

    visitor->onFinishVisit(this);
};

ShortVarDeclarationStatement* ShortVarDeclarationStatement::clone() const noexcept {
    ExpressionList cloneValues;

    for (auto expr : this->values) {
        cloneValues.push_back(expr->clone());
    }

    IdentifiersList cloneIdentifiers;

    for (auto id : this->identifiers) {
        cloneIdentifiers.push_back(id);
    }

    return new ShortVarDeclarationStatement(cloneIdentifiers, cloneValues);
};