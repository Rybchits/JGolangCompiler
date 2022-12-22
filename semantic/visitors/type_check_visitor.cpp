#include "./type_check_visitor.h"
#include <unordered_map>
#include <iostream>

bool TypeCheckVisitor::check(NodeAST* root) {
    std::cout << "Start type check" << std::endl;
    scopes.push_back(std::unordered_map<std::string, JavaType>());
    root->acceptVisitor(this);
    return true;
}

void TypeCheckVisitor::onStartVisit(BlockStatement* node) {
    if (!lastAddedScopeInFuncDecl) {
        scopes.push_back(std::unordered_map<std::string, JavaType>());
    }
    lastAddedScopeInFuncDecl = false;
}

void TypeCheckVisitor::onFinishVisit(BlockStatement* node) {
    scopes.pop_back();
}

void TypeCheckVisitor::onStartVisit(FunctionDeclaration* node) {
    scopes.push_back(std::unordered_map<std::string, JavaType>());
    lastAddedScopeInFuncDecl = true;
    
}

void TypeCheckVisitor::onFinishVisit(VariableDeclaration* node) {
    // Добавить все объявленные переменные в таблицу скоупа
    // Todo если нет явно указанного типа присвоить из values
    for (auto id : node->identifiersWithType->identifiers) {
        scopes.back()[id] = JavaType(node->identifiersWithType->type);
        std::cout << id << ": index scope" << scopes.size() << std::endl;
    }
}

void TypeCheckVisitor::onFinishVisit(ShortVarDeclarationStatement* node) {
    for (auto id : node->identifiers) {
        scopes.back()[id] = JavaType();
        std::cout << id << ": index scope" << scopes.size() << std::endl;
    }
}

void TypeCheckVisitor::onFinishVisit(IdentifierAsExpression* node) {
    for (auto scope : scopes) {
        if (scope.count(node->identifier)) {
            typesNode[node->nodeId] = scope[node->identifier];
            return ;
        }
    }

    typesNode[node->nodeId] = JavaType();
}


void TypeCheckVisitor::onFinishVisit(IntegerExpression* node) {
    typesNode[node->nodeId] = JavaType("int");
}

void TypeCheckVisitor::onFinishVisit(BooleanExpression* node) {
    typesNode[node->nodeId] = JavaType("bool");
}

void TypeCheckVisitor::onFinishVisit(FloatExpression* node) {
    typesNode[node->nodeId] = JavaType("float64");
}

void TypeCheckVisitor::onFinishVisit(StringExpression* node) {
    typesNode[node->nodeId] = JavaType("string");
}

void TypeCheckVisitor::onFinishVisit(RuneExpression* node) {
    typesNode[node->nodeId] = JavaType("rune");
}

void TypeCheckVisitor::onFinishVisit(NilExpression* node) {
    typesNode[node->nodeId] = JavaType();
}

void TypeCheckVisitor::onFinishVisit(UnaryExpression* node) {
    typesNode[node->nodeId] = typesNode[node->expression->nodeId];
}

void TypeCheckVisitor::onFinishVisit(BinaryExpression* node) {
    /*if (typesNode[node->lhs->nodeId] == typesNode[node->rhs->nodeId]) {
        typesNode[node->nodeId] = typesNode[node->lhs->nodeId];
    } else {
        throw ProcessingInterrupted("Mismatched types in " + node->name());
    }*/

    if (node->type == Addition || node->type == Subtraction  || node->type == Multiplication
        || node->type == Division || node->type == Mod
    ) {

    } else if (node->type == Or || node->type == And) {

    } else {

    }
}

void TypeCheckVisitor::onFinishVisit(CallableExpression* node) {

}

void TypeCheckVisitor::onFinishVisit(AccessExpression* node) {

}

void TypeCheckVisitor::onFinishVisit(CompositeLiteral* node) {

}

