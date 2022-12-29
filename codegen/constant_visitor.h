#pragma once

#include "../visitor.h"
#include "./constant.h"


class ConstantVisitor : public Visitor {
private:
    ConstantPool* pool;

    void onStartVisit(StringExpression* node) override;
    void onStartVisit(IntegerExpression* node) override;
    void onStartVisit(FloatExpression* node) override;

public:
    explicit ConstantVisitor(ConstantPool* pool) : pool(pool) {};
    
    void getConstants(ExpressionAST* node);
    void getConstants(BlockStatement* node);

    ~ConstantVisitor() override = default;
};