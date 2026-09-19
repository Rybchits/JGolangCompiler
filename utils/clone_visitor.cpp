#include "clone_visitor.h"

void CloneVisitor::onFinishVisit(NodeAST& node) {
    throw std::logic_error("Unsupported AST node " + std::to_string(node.nodeId));
}

void CloneVisitor::onFinishVisit(PackageAST& node) {
    store(node, std::make_unique<PackageAST>(node.packageName, takeList(node.topDeclarations)));
}

void CloneVisitor::onFinishVisit(VariableDeclaration& node) {
    store(node, std::make_unique<VariableDeclaration>(takeChild(node.identifiersWithType), takeList(node.values), node.isConst));
}

void CloneVisitor::onFinishVisit(TypeDeclaration& node) {
    store(node, std::make_unique<TypeDeclaration>(node.alias, takeChild(node.declType)));
}

void CloneVisitor::onFinishVisit(FunctionDeclaration& node) {
    store(node, std::make_unique<FunctionDeclaration>(node.identifier, takeChild(node.signature), takeChild(node.block)));
}

void CloneVisitor::onFinishVisit(MethodDeclaration& node) {
    store(node, std::make_unique<MethodDeclaration>(node.identifier, node.receiverIdentifier, takeChild(node.receiverType),
        takeChild(node.signature), takeChild(node.block)));
}

void CloneVisitor::onFinishVisit(IdentifierAsExpression& node) {
    auto copy = std::make_unique<IdentifierAsExpression>(node.identifier);
    copy->isDestination = node.isDestination;
    store(node, std::move(copy));
}

void CloneVisitor::onFinishVisit(IntegerExpression& node) {
    store(node, std::make_unique<IntegerExpression>(node.intLit));
}

void CloneVisitor::onFinishVisit(BooleanExpression& node) {
    store(node, std::make_unique<BooleanExpression>(node.boolLit));
}

void CloneVisitor::onFinishVisit(FloatExpression& node) {
    store(node, std::make_unique<FloatExpression>(node.floatLit));
}

void CloneVisitor::onFinishVisit(StringExpression& node) {
    store(node, std::make_unique<StringExpression>(node.stringLit));
}

void CloneVisitor::onFinishVisit(NilExpression& node) {
    store(node, std::make_unique<NilExpression>());
}

void CloneVisitor::onFinishVisit(FunctionLitExpression& node) {
    store(node, std::make_unique<FunctionLitExpression>(takeChild(node.signature), takeChild(node.block)));
}

void CloneVisitor::onFinishVisit(UnaryExpression& node) {
    store(node, std::make_unique<UnaryExpression>(node.type, takeChild(node.expression)));
}

void CloneVisitor::onFinishVisit(BinaryExpression& node) {
    store(node, std::make_unique<BinaryExpression>(node.type, takeChild(node.lhs), takeChild(node.rhs)));
}

void CloneVisitor::onFinishVisit(CallableExpression& node) {
    store(node, std::make_unique<CallableExpression>(takeChild(node.base), takeList(node.arguments)));
}

void CloneVisitor::onFinishVisit(AccessExpression& node) {
    store(node, std::make_unique<AccessExpression>(node.type, takeChild(node.base), takeChild(node.accessor)));
}

void CloneVisitor::onFinishVisit(ElementCompositeLiteral& node) {
    auto key = takeChild(node.key);
    if (const auto* expression = std::get_if<ExpressionASTPtr>(&node.value)) {
        store(node, std::make_unique<ElementCompositeLiteral>(std::move(key), takeChild(*expression)));
    } else {
        store(node, std::make_unique<ElementCompositeLiteral>(
            std::move(key), takeList(std::get<ElementCompositeLiteralList>(node.value))));
    }
}

void CloneVisitor::onFinishVisit(CompositeLiteral& node) {
    store(node, std::make_unique<CompositeLiteral>(takeChild(node.type), takeList(node.elements)));
}

void CloneVisitor::onFinishVisit(BlockStatement& node) {
    store(node, std::make_unique<BlockStatement>(takeList(node.body)));
}

void CloneVisitor::onFinishVisit(KeywordStatement& node) {
    store(node, std::make_unique<KeywordStatement>(node.type));
}

void CloneVisitor::onFinishVisit(ExpressionStatement& node) {
    store(node, std::make_unique<ExpressionStatement>(takeChild(node.expression)));
}

void CloneVisitor::onFinishVisit(AssignmentStatement& node) {
    auto copy = std::make_unique<AssignmentStatement>(node.type, ExpressionList{}, ExpressionList{});
    copy->lhs = takeList(node.lhs);
    copy->rhs = takeList(node.rhs);
    copy->indexes = takeList(node.indexes);
    store(node, std::move(copy));
}

void CloneVisitor::onFinishVisit(ForStatement& node) {
    store(node, std::make_unique<ForStatement>(takeChild(node.initStatement), takeChild(node.conditionExpression),
        takeChild(node.iterationStatement), takeChild(node.block)));
}

void CloneVisitor::onFinishVisit(WhileStatement& node) {
    store(node, std::make_unique<WhileStatement>(takeChild(node.conditionExpression), takeChild(node.block)));
}

void CloneVisitor::onFinishVisit(ForRangeStatement& node) {
    store(node, std::make_unique<ForRangeStatement>(takeList(node.initStatement), takeChild(node.expressionValue),
        takeChild(node.block), node.hasShortDeclaration));
}

void CloneVisitor::onFinishVisit(ReturnStatement& node) {
    store(node, std::make_unique<ReturnStatement>(takeList(node.returnValues)));
}

void CloneVisitor::onFinishVisit(IfStatement& node) {
    store(node, std::make_unique<IfStatement>(takeChild(node.preStatement), takeChild(node.condition),
        takeChild(node.thenStatement), takeChild(node.elseStatement)));
}

void CloneVisitor::onFinishVisit(SwitchCaseClause& node) {
    auto copy = std::make_unique<SwitchCaseClause>(takeChild(node.expressionCase), takeChild(node.block));
    copy->fallthrowEnds = node.fallthrowEnds;
    store(node, std::move(copy));
}

void CloneVisitor::onFinishVisit(SwitchStatement& node) {
    store(node, std::make_unique<SwitchStatement>(takeChild(node.statement), takeChild(node.expression), takeList(node.clauseList)));
}

void CloneVisitor::onFinishVisit(DeclarationStatement& node) {
    store(node, std::make_unique<DeclarationStatement>(takeList(node.declarations)));
}

void CloneVisitor::onFinishVisit(IdentifiersWithType& node) {
    store(node, std::make_unique<IdentifiersWithType>(node.identifiers, takeChild(node.type)));
}

void CloneVisitor::onFinishVisit(FunctionSignature& node) {
    store(node, std::make_unique<FunctionSignature>(takeList(node.idsAndTypesArgs), takeList(node.idsAndTypesResults)));
}

void CloneVisitor::onFinishVisit(ArraySignature& node) {
    store(node, std::make_unique<ArraySignature>(takeChild(node.arrayElementType), node.dimensions));
}

void CloneVisitor::onFinishVisit(IdentifierAsType& node) {
    store(node, std::make_unique<IdentifierAsType>(node.identifier));
}

void CloneVisitor::onFinishVisit(StructSignature& node) {
    store(node, std::make_unique<StructSignature>(takeList(node.structMembers)));
}

void CloneVisitor::onFinishVisit(InterfaceType& node) {
    store(node, std::make_unique<InterfaceType>(takeList(node.functions)));
}

void CloneVisitor::onFinishVisit(ShortVarDeclarationStatement& node) {
    store(node, std::make_unique<ShortVarDeclarationStatement>(node.identifiers, takeList(node.values)));
}
