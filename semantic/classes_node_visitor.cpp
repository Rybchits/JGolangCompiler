#include "./classes_node_visitor.h"

std::string ClassesNodeVisitor::getNameCurrentClass() {
    return "$class_" + std::to_string(indexClassInDeclaration++);
}

void ClassesNodeVisitor::visit(NodeAST *node) {}

void ClassesNodeVisitor::visit(IdentifiersWithType *node) {
    if (dynamic_cast<StructSignature*>(node->type) || dynamic_cast<InterfaceType*>(node->type)) {
        std::string name = getNameCurrentClass();
        classes[name] = JavaClass(node->type);
        node->type = new IdentifierAsType(name);
    }
}

void ClassesNodeVisitor::visit(ArraySignature *node) {
    if (dynamic_cast<StructSignature*>(node->arrayElementType) || dynamic_cast<InterfaceType*>(node->arrayElementType)) {
        std::string name = getNameCurrentClass();
        classes[name] = JavaClass(node->arrayElementType);
        node->arrayElementType = new IdentifierAsType(name);
    }
}

void ClassesNodeVisitor::visit(CompositeLiteral *node) {
    if (dynamic_cast<StructSignature*>(node->type) || dynamic_cast<InterfaceType*>(node->type)) {
        std::string name = getNameCurrentClass();
        classes[name] = JavaClass(node->type);
        node->type = new IdentifierAsType(name);
    }
}
