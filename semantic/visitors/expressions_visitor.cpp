#include "expressions_visitor.h"

void ExpressionsVisitor::transform(PackageAST* packageAst) {
    packageAst->acceptVisitor(this);
}

ExpressionAST* ExpressionsVisitor::transformUnaryExpression(UnaryExpression* expr) {
    ExpressionAST* result = expr;

    switch (expr->type) {
    case UnaryMinus:
        if (auto integerExpr = dynamic_cast<IntegerExpression*>(expr->expression)) {
            integerExpr->intLit = -(integerExpr->intLit);
            result = integerExpr;

        } else if (auto floatingExpr = dynamic_cast<FloatExpression*>(expr->expression)) {
            floatingExpr->floatLit = -(floatingExpr->floatLit);
            result = floatingExpr;
        }
        break;

    case UnaryPlus:
        if (auto integerExpr = dynamic_cast<IntegerExpression*>(expr->expression))
            result = integerExpr;

        else if (auto floatingExpr = dynamic_cast<FloatExpression*>(expr->expression))
            result = floatingExpr;

        break;

    case UnaryNot:
        if (auto booleanExpr = dynamic_cast<BooleanExpression*>(expr->expression)) {
            booleanExpr->boolLit = !(booleanExpr->boolLit);
            result = booleanExpr;
        }
        break;

    default:
        break;
    }

    return result;
}

ExpressionAST* ExpressionsVisitor::transformBinaryExpression(BinaryExpression* expr) {
    ExpressionAST* result = expr;

    auto leftInt = dynamic_cast<IntegerExpression*>(expr->lhs);
    auto leftFloat = dynamic_cast<FloatExpression*>(expr->lhs);
    auto leftBool = dynamic_cast<BooleanExpression*>(expr->lhs);
    auto leftString = dynamic_cast<StringExpression*>(expr->lhs);

    auto rightInt = dynamic_cast<IntegerExpression*>(expr->rhs);
    auto rightFloat = dynamic_cast<FloatExpression*>(expr->rhs);
    auto rightBool = dynamic_cast<BooleanExpression*>(expr->rhs);
    auto rightString = dynamic_cast<StringExpression*>(expr->rhs);

    switch (expr->type)
    {
    case Addition:
        if (leftInt && rightInt)
            result = new IntegerExpression(leftInt->intLit + rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new FloatExpression(leftFloat->floatLit + rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new FloatExpression(leftInt->intLit + rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new FloatExpression(leftFloat->floatLit + rightFloat->floatLit);
        
        else if (leftString && rightString)
            result = new StringExpression(leftString->stringLit + rightString->stringLit);

        break;
    
    case Subtraction:
        if (leftInt && rightInt)
            result = new IntegerExpression(leftInt->intLit - rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new FloatExpression(leftFloat->floatLit - rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new FloatExpression(leftInt->intLit - rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new FloatExpression(leftFloat->floatLit - rightFloat->floatLit);

        break;
    
    case Division:
        if (leftInt && rightInt)
            result = new IntegerExpression(leftInt->intLit / rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new FloatExpression(leftFloat->floatLit / rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new FloatExpression(leftInt->intLit / rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new FloatExpression(leftFloat->floatLit / rightFloat->floatLit);

        break;
    
    case Multiplication:
        if (leftInt && rightInt)
            result = new IntegerExpression(leftInt->intLit * rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new FloatExpression(leftFloat->floatLit * rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new FloatExpression(leftInt->intLit * rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new FloatExpression(leftFloat->floatLit * rightFloat->floatLit);

        break;
    
    case And:
        if (leftBool && rightBool)
            result = new BooleanExpression(leftBool->boolLit && rightBool->boolLit);

        break;
    
    case Or:
        if (leftBool && rightBool)
            result = new BooleanExpression(leftBool->boolLit || rightBool->boolLit);
        break;

    case Mod:
        if (leftInt && rightInt)
            result = new IntegerExpression(leftInt->intLit % rightInt->intLit);

        break;

    case Equal:
        if (leftInt && rightInt)
            result = new BooleanExpression(leftInt->intLit == rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new BooleanExpression(leftFloat->floatLit == rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new BooleanExpression(leftInt->intLit == rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new BooleanExpression(leftFloat->floatLit == rightFloat->floatLit);

        else if (leftBool && rightBool)
            result = new BooleanExpression(leftBool->boolLit == rightBool->boolLit);

        else if (leftString && rightString)
            result = new BooleanExpression(leftString->stringLit == rightString->stringLit);

        break;

    case Greater:
        if (leftInt && rightInt)
            result = new BooleanExpression(leftInt->intLit > rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new BooleanExpression(leftFloat->floatLit > rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new BooleanExpression(leftInt->intLit > rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new BooleanExpression(leftFloat->floatLit > rightFloat->floatLit);

        else if (leftString && rightString)
            result = new BooleanExpression(leftString->stringLit > rightString->stringLit);

        break;

    case Less:
        if (leftInt && rightInt)
            result = new BooleanExpression(leftInt->intLit < rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new BooleanExpression(leftFloat->floatLit < rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new BooleanExpression(leftInt->intLit < rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new BooleanExpression(leftFloat->floatLit < rightFloat->floatLit);

        else if (leftString && rightString)
            result = new BooleanExpression(leftString->stringLit < rightString->stringLit);

        break;

    case NotEqual:
        if (leftInt && rightInt)
            result = new BooleanExpression(leftInt->intLit != rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new BooleanExpression(leftFloat->floatLit != rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new BooleanExpression(leftInt->intLit != rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new BooleanExpression(leftFloat->floatLit != rightFloat->floatLit);

        else if (leftBool && rightBool)
            result = new BooleanExpression(leftBool->boolLit != rightBool->boolLit);

        else if (leftString && rightString)
            result = new BooleanExpression(leftString->stringLit != rightString->stringLit);

        break;

    case LessOrEqual:
        if (leftInt && rightInt)
            result = new BooleanExpression(leftInt->intLit <= rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new BooleanExpression(leftFloat->floatLit <= rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new BooleanExpression(leftInt->intLit <= rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new BooleanExpression(leftFloat->floatLit <= rightFloat->floatLit);

        else if (leftString && rightString)
            result = new BooleanExpression(leftString->stringLit <= rightString->stringLit);

        break;

    case GreatOrEqual:
        if (leftInt && rightInt)
            result = new BooleanExpression(leftInt->intLit >= rightInt->intLit);

        else if (leftFloat && rightInt)
            result = new BooleanExpression(leftFloat->floatLit >= rightInt->intLit);

        else if (leftInt && rightFloat)
            result = new BooleanExpression(leftInt->intLit >= rightFloat->floatLit);

        else if (leftFloat && rightFloat)
            result = new BooleanExpression(leftFloat->floatLit >= rightFloat->floatLit);

        else if (leftString && rightString)
            result = new BooleanExpression(leftString->stringLit >= rightString->stringLit);

        break;
    }

    return result;
}

ExpressionAST* ExpressionsVisitor::transformExpression(ExpressionAST* expr) {
    ExpressionAST* transofrmedExpression = expr;

    if (auto unary = dynamic_cast<UnaryExpression*>(expr)) {
        transofrmedExpression = transformUnaryExpression(unary);

    } else if (auto binary = dynamic_cast<BinaryExpression*>(expr)) {
        transofrmedExpression = transformBinaryExpression(binary);
    }

    return transofrmedExpression;
}

ExpressionList ExpressionsVisitor::transformExpressionList(ExpressionList& list) {
    ExpressionList transofrmedList;

    for (auto expr : list) {
        transofrmedList.push_back(transformExpression(expr));
    }

    return transofrmedList;
}

void ExpressionsVisitor::onFinishVisit(VariableDeclaration* node) {
    node->values = transformExpressionList(node->values);
}

void ExpressionsVisitor::onFinishVisit(UnaryExpression* node) {
    node->expression = transformExpression(node->expression);
}

void ExpressionsVisitor::onFinishVisit(BinaryExpression* node) {
    node->lhs = transformExpression(node->lhs);
    node->rhs = transformExpression(node->rhs);
}

void ExpressionsVisitor::onFinishVisit(CallableExpression* node) {
    node->base = transformExpression(node->base);
    node->arguments = transformExpressionList(node->arguments);
}

void ExpressionsVisitor::onFinishVisit(AccessExpression* node) {
    node->base = transformExpression(node->base);
    node->accessor = transformExpression(node->accessor);
}

void ExpressionsVisitor::onFinishVisit(ElementCompositeLiteral* node) {
    node->key = transformExpression(node->key);

    if (std::holds_alternative<ExpressionAST *>(node->value)) {
        node->value = transformExpression(std::get<ExpressionAST *>(node->value));
    }
}

void ExpressionsVisitor::onFinishVisit(ExpressionStatement* node) {
    node->expression = transformExpression(node->expression);
}

void ExpressionsVisitor::onFinishVisit(ReturnStatement* node) {
    node->returnValues = transformExpressionList(node->returnValues);
}

void ExpressionsVisitor::onFinishVisit(AssignmentStatement* node) {
    node->indexes = transformExpressionList(node->indexes);
    node->lhs = transformExpressionList(node->lhs);
    node->rhs = transformExpressionList(node->rhs);
}

void ExpressionsVisitor::onFinishVisit(ForStatement* node) {
    node->conditionExpression = transformExpression(node->conditionExpression);
}

void ExpressionsVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    node->values = transformExpressionList(node->values);
}

void ExpressionsVisitor::onFinishVisit(WhileStatement* node) {
    node->conditionExpression = transformExpression(node->conditionExpression);
}

void ExpressionsVisitor::onFinishVisit(IfStatement* node) {
    node->condition = transformExpression(node->condition);
}

void ExpressionsVisitor::onFinishVisit(SwitchStatement* node) {
    node->expression = transformExpression(node->expression);
}

void ExpressionsVisitor::onFinishVisit(SwitchCaseClause* node) {
    node->expressionCase = transformExpression(node->expressionCase);
}
