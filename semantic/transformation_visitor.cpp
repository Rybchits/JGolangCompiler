#include "./transformation_visitor.h"

std::string TreeTransformationVisitor::getNameCurrentClass() {
    return "$class_" + std::to_string(indexClassInDeclaration++);
}


StatementList TreeTransformationVisitor::transformForToWhile(ForStatement *forStmt) {
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

    return list;
}


StatementList TreeTransformationVisitor::transformForRangeToWhile(ForRangeStatement *forRangeStmt) {
    StatementList list;
    std::string indexPrivateVariableName = "$index";

    auto indexDeclaration = new VariableDeclaration(
            new IdentifiersWithType(
                    *(new IdentifiersList{indexPrivateVariableName}), new IdentifierAsType("int")),
            *(new ExpressionList{new IntegerExpression(0)})
    );

    list.push_back(new DeclarationStatement(*(new DeclarationList{indexDeclaration})));

    auto condition = new BinaryExpression(
            BinaryExpressionEnum::Less,
            new IdentifierAsExpression(indexPrivateVariableName),
            new CallableExpression(new IdentifierAsExpression("len"),
                                   *(new ExpressionList{forRangeStmt->expressionValue}))
    );

    if (forRangeStmt->initStatement.size() > 2) {
        semantic->errors.push_back("Many variables in initialization ForRange loop");
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
                semantic->errors.push_back("Undefined expression (1) in initialization ForRange loop");
            }
        } else {
            indexVariableStatement = new AssignmentStatement(
                    AssignmentEnum::SimpleAssign,
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
                AccessExpressionEnum::Indexing,
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
                semantic->errors.push_back("Undefined expression (2) in initialization ForRange loop");
            }
        } else {
            elementVariableStatement = new AssignmentStatement(
                    AssignmentEnum::SimpleAssign,
                    *variableForRange,
                    accessToElement);
        }

        forRangeStmt->block->body.insert(forRangeStmt->block->body.begin(), elementVariableStatement);
    }

    forRangeStmt->block->body.insert(forRangeStmt->block->body.end(), new ExpressionStatement(
            new UnaryExpression(
                    UnaryExpressionEnum::Increment,
                    new IdentifierAsExpression(indexPrivateVariableName))));

    list.push_back(new WhileStatement(condition, forRangeStmt->block));
    return list;
}

void TreeTransformationVisitor::visit(IdentifiersWithType *node) {
    if (dynamic_cast<StructSignature *>(node->type) || dynamic_cast<InterfaceType *>(node->type)) {
        std::string name = getNameCurrentClass();
        classes[name] = JavaClass(node->type);
        node->type = new IdentifierAsType(name);
    }
}

void TreeTransformationVisitor::visit(ArraySignature *node) {
    if (dynamic_cast<StructSignature *>(node->arrayElementType) ||
        dynamic_cast<InterfaceType *>(node->arrayElementType)) {
        std::string name = getNameCurrentClass();
        classes[name] = JavaClass(node->arrayElementType);
        node->arrayElementType = new IdentifierAsType(name);
    }
}

void TreeTransformationVisitor::visit(CompositeLiteral *node) {
    if (dynamic_cast<StructSignature *>(node->type) || dynamic_cast<InterfaceType *>(node->type)) {
        std::string name = getNameCurrentClass();
        classes[name] = JavaClass(node->type);
        node->type = new IdentifierAsType(name);
    }
}

void TreeTransformationVisitor::visit(BlockStatement *node) {
    StatementList newBody;

    for (auto stmt: node->body) {
        StatementList transformedLoop;

        if (auto forLoop = dynamic_cast<ForStatement *>(stmt)) {
            transformedLoop = transformForToWhile(forLoop);

        } else if (auto forRangeLoop = dynamic_cast<ForRangeStatement *>(stmt)) {
            transformedLoop = transformForRangeToWhile(forRangeLoop);

        } else {
            newBody.push_back(stmt);
            continue;
        }

        newBody.insert(newBody.end(), transformedLoop.begin(), transformedLoop.end());
    }
    node->body = newBody;
}


std::unordered_map<std::string, JavaClass> TreeTransformationVisitor::transform(PackageAST* packageAst) {
    packageAst->acceptVisitor(this, TraversalMethod::Upward);
    auto returnClassesMap = classes;
    classes = std::unordered_map<std::string, JavaClass>();
    return returnClassesMap;
}