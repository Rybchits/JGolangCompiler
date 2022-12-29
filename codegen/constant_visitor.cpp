#include "constant_visitor.h"

void ConstantVisitor::onStartVisit(StringExpression* node) {
    pool->FindUtf8("Ljava/lang/String;");
    pool->FindString(node->stringLit);
}

void ConstantVisitor::onStartVisit(IntegerExpression* node) {
    if (node->intLit < -1 || node->intLit > 5) {
        pool->FindInt(node->intLit);
    }
}

void ConstantVisitor::onStartVisit(FloatExpression* node) {
    pool->FindFloat(node->floatLit);
}

void ConstantVisitor::getConstants(BlockStatement* node) {
    node->acceptVisitor(this);    
}

void ConstantVisitor::getConstants(ExpressionAST* node) {
    node->acceptVisitor(this);    
}

