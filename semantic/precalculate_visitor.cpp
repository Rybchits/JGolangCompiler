#include "precalculate_visitor.h"

void PrecalculateVisitor::transform(PackageAST* packageAst) {
    packageAst->acceptVisitor(this);
}

ExpressionASTPtr PrecalculateVisitor::transformUnaryExpression(UnaryExpression* expr) {
    ExpressionASTPtr result;

    switch (expr->type) {
    case UnaryExpression::UnaryMinus:
        if (auto integerExpr = dynamic_cast<IntegerExpression*>(expr->expression.get())) {
            integerExpr->intLit = -(integerExpr->intLit);
            result = std::move(expr->expression);

        } else if (auto floatingExpr = dynamic_cast<FloatExpression*>(expr->expression.get())) {
            floatingExpr->floatLit = -(floatingExpr->floatLit);
            result = std::move(expr->expression);
        }
        break;

    case UnaryExpression::UnaryPlus:
        if (auto integerExpr = dynamic_cast<IntegerExpression*>(expr->expression.get()))
            result = std::move(expr->expression);

        else if (auto floatingExpr = dynamic_cast<FloatExpression*>(expr->expression.get()))
            result = std::move(expr->expression);

        break;

    case UnaryExpression::UnaryNot:
        if (auto booleanExpr = dynamic_cast<BooleanExpression*>(expr->expression.get())) {
            booleanExpr->boolLit = !(booleanExpr->boolLit);
            result = std::move(expr->expression);
        }
        break;

    default:
        break;
    }

    return result;
}

ExpressionASTPtr PrecalculateVisitor::transformBinaryExpression(BinaryExpression* expr) {
    ExpressionASTPtr result;

    auto leftInt = dynamic_cast<IntegerExpression*>(expr->lhs.get());
    auto leftFloat = dynamic_cast<FloatExpression*>(expr->lhs.get());
    auto leftBool = dynamic_cast<BooleanExpression*>(expr->lhs.get());
    auto leftString = dynamic_cast<StringExpression*>(expr->lhs.get());

    auto rightInt = dynamic_cast<IntegerExpression*>(expr->rhs.get());
    auto rightFloat = dynamic_cast<FloatExpression*>(expr->rhs.get());
    auto rightBool = dynamic_cast<BooleanExpression*>(expr->rhs.get());
    auto rightString = dynamic_cast<StringExpression*>(expr->rhs.get());

    switch (expr->type)
    {
    case BinaryExpression::Addition:
        if (leftInt && rightInt)
            result = std::make_unique<IntegerExpression>(leftInt->intLit + rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit + rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<FloatExpression>(leftInt->intLit + rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit + rightFloat->floatLit);
        
        else if (leftString && rightString)
            result = std::make_unique<StringExpression>(leftString->stringLit + rightString->stringLit);

        break;
    
    case BinaryExpression::Subtraction:
        if (leftInt && rightInt)
            result = std::make_unique<IntegerExpression>(leftInt->intLit - rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit - rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<FloatExpression>(leftInt->intLit - rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit - rightFloat->floatLit);

        break;
    
    case BinaryExpression::Division:
        if (leftInt && rightInt)
            result = std::make_unique<IntegerExpression>(leftInt->intLit / rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit / rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<FloatExpression>(leftInt->intLit / rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit / rightFloat->floatLit);

        break;
    
    case BinaryExpression::Multiplication:
        if (leftInt && rightInt)
            result = std::make_unique<IntegerExpression>(leftInt->intLit * rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit * rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<FloatExpression>(leftInt->intLit * rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<FloatExpression>(leftFloat->floatLit * rightFloat->floatLit);

        break;
    
    case BinaryExpression::And:
        if (leftBool && rightBool)
            result = std::make_unique<BooleanExpression>(leftBool->boolLit && rightBool->boolLit);

        break;
    
    case BinaryExpression::Or:
        if (leftBool && rightBool)
            result = std::make_unique<BooleanExpression>(leftBool->boolLit || rightBool->boolLit);
        break;

    case BinaryExpression::Mod:
        if (leftInt && rightInt)
            result = std::make_unique<IntegerExpression>(leftInt->intLit % rightInt->intLit);

        break;

    case BinaryExpression::Equal:
        if (leftInt && rightInt)
            result = std::make_unique<BooleanExpression>(leftInt->intLit == rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit == rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<BooleanExpression>(leftInt->intLit == rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit == rightFloat->floatLit);

        else if (leftBool && rightBool)
            result = std::make_unique<BooleanExpression>(leftBool->boolLit == rightBool->boolLit);

        else if (leftString && rightString)
            result = std::make_unique<BooleanExpression>(leftString->stringLit == rightString->stringLit);

        break;

    case BinaryExpression::Greater:
        if (leftInt && rightInt)
            result = std::make_unique<BooleanExpression>(leftInt->intLit > rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit > rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<BooleanExpression>(leftInt->intLit > rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit > rightFloat->floatLit);

        else if (leftString && rightString)
            result = std::make_unique<BooleanExpression>(leftString->stringLit > rightString->stringLit);

        break;

    case BinaryExpression::Less:
        if (leftInt && rightInt)
            result = std::make_unique<BooleanExpression>(leftInt->intLit < rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit < rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<BooleanExpression>(leftInt->intLit < rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit < rightFloat->floatLit);

        else if (leftString && rightString)
            result = std::make_unique<BooleanExpression>(leftString->stringLit < rightString->stringLit);

        break;

    case BinaryExpression::NotEqual:
        if (leftInt && rightInt)
            result = std::make_unique<BooleanExpression>(leftInt->intLit != rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit != rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<BooleanExpression>(leftInt->intLit != rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit != rightFloat->floatLit);

        else if (leftBool && rightBool)
            result = std::make_unique<BooleanExpression>(leftBool->boolLit != rightBool->boolLit);

        else if (leftString && rightString)
            result = std::make_unique<BooleanExpression>(leftString->stringLit != rightString->stringLit);

        break;

    case BinaryExpression::LessOrEqual:
        if (leftInt && rightInt)
            result = std::make_unique<BooleanExpression>(leftInt->intLit <= rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit <= rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<BooleanExpression>(leftInt->intLit <= rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit <= rightFloat->floatLit);

        else if (leftString && rightString)
            result = std::make_unique<BooleanExpression>(leftString->stringLit <= rightString->stringLit);

        break;

    case BinaryExpression::GreatOrEqual:
        if (leftInt && rightInt)
            result = std::make_unique<BooleanExpression>(leftInt->intLit >= rightInt->intLit);

        else if (leftFloat && rightInt)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit >= rightInt->intLit);

        else if (leftInt && rightFloat)
            result = std::make_unique<BooleanExpression>(leftInt->intLit >= rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = std::make_unique<BooleanExpression>(leftFloat->floatLit >= rightFloat->floatLit);

        else if (leftString && rightString)
            result = std::make_unique<BooleanExpression>(leftString->stringLit >= rightString->stringLit);

        break;
    }

    return result;
}

ExpressionASTPtr PrecalculateVisitor::transformExpression(ExpressionASTPtr expr) {
    ExpressionASTPtr replacement;
    if (auto* unary = dynamic_cast<UnaryExpression*>(expr.get())) {
        replacement = transformUnaryExpression(unary);
    } else if (auto* binary = dynamic_cast<BinaryExpression*>(expr.get())) {
        replacement = transformBinaryExpression(binary);
    }
    return replacement ? std::move(replacement) : std::move(expr);
}

void PrecalculateVisitor::transformExpressionList(ExpressionList& list) {
    for (auto& expr : list) {
        expr = transformExpression(std::move(expr));
    }
}

void PrecalculateVisitor::onFinishVisit(VariableDeclaration* node) {
    transformExpressionList(node->values);
}

void PrecalculateVisitor::onFinishVisit(UnaryExpression* node) {
    node->expression = transformExpression(std::move(node->expression));
}

void PrecalculateVisitor::onFinishVisit(BinaryExpression* node) {
    node->lhs = transformExpression(std::move(node->lhs));
    node->rhs = transformExpression(std::move(node->rhs));
}

void PrecalculateVisitor::onFinishVisit(CallableExpression* node) {
    node->base = transformExpression(std::move(node->base));
    transformExpressionList(node->arguments);
}

void PrecalculateVisitor::onFinishVisit(AccessExpression* node) {
    node->base = transformExpression(std::move(node->base));
    node->accessor = transformExpression(std::move(node->accessor));
}

void PrecalculateVisitor::onFinishVisit(ElementCompositeLiteral* node) {
    node->key = transformExpression(std::move(node->key));

    if (std::holds_alternative<ExpressionASTPtr>(node->value)) {
        node->value = transformExpression(std::move(std::get<ExpressionASTPtr>(node->value)));
    }
}

void PrecalculateVisitor::onFinishVisit(ExpressionStatement* node) {
    node->expression = transformExpression(std::move(node->expression));
}

void PrecalculateVisitor::onFinishVisit(ReturnStatement* node) {
    transformExpressionList(node->returnValues);
}

void PrecalculateVisitor::onFinishVisit(AssignmentStatement* node) {
    transformExpressionList(node->indexes);
    transformExpressionList(node->lhs);
    transformExpressionList(node->rhs);
}

void PrecalculateVisitor::onFinishVisit(ForStatement* node) {
    node->conditionExpression = transformExpression(std::move(node->conditionExpression));
}

void PrecalculateVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    transformExpressionList(node->values);
}

void PrecalculateVisitor::onFinishVisit(WhileStatement* node) {
    node->conditionExpression = transformExpression(std::move(node->conditionExpression));
}

void PrecalculateVisitor::onFinishVisit(IfStatement* node) {
    node->condition = transformExpression(std::move(node->condition));
}

void PrecalculateVisitor::onFinishVisit(SwitchStatement* node) {
    node->expression = transformExpression(std::move(node->expression));
}

void PrecalculateVisitor::onFinishVisit(SwitchCaseClause* node) {
    node->expressionCase = transformExpression(std::move(node->expressionCase));
}
