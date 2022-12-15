#include "./java_entity_visitor.h"
/*
void JavaEntityVisitor::visit(const IdentifiersWithType* node) {
    std::vector<JavaVariable> variableList;

    JavaType* type;
    if (node->type != nullptr) {
        //type = createJavaType(node->type);
    }
    
    for (std::string id : node->identifiers) {
        variableList.push_back(JavaVariable(id, type));
    }

    javaEntityStack.push(variableList);
}

void JavaEntityVisitor::visit(const IdentifierAsType* node) {
    javaEntityStack.push(JavaType(node->identifier));
}




void JavaEntityVisitor::visit(const StructSignature* node) {
    /*JavaClass javaClass;

    for (auto membersLine : node->structMembers) {
        auto fields = createJavaVariable(membersLine);
        javaClass.addField(std::move(fields));
    }

    javaEntityStack.push(javaClass);*/
// }


// void JavaEntityVisitor::visit(const ArraySignature* node) {
//     //javaEntityStack.push(JavaArraySignature(node->dimensions, createJavaType(node->arrayElementType)));
// }


// void JavaEntityVisitor::visit(const FunctionSignature* node) {
//     //JavaMethod javaMethod;
// }

// void JavaEntityVisitor::visit(const MethodDeclaration* node) {

// }

// void JavaEntityVisitor::visit(const BlockStatement* node) {
//     throw ProcessingInterrupted();
// }

// std::vector<JavaVariable> JavaEntityVisitor::createJavaVariable(const IdentifiersWithType* typedIdsNode) {
//     typedIdsNode->acceptVisitor(this);
//     auto javaVariableList = std::any_cast<std::vector<JavaVariable>>(javaEntityStack.top());
//     javaEntityStack.pop();
//     return javaVariableList;
// }

// JavaMethod JavaEntityVisitor::createJavaMethod(const FunctionDeclaration* functionNode) {
//     functionNode->acceptVisitor(this);
//     auto javaMethod = std::any_cast<JavaMethod>(javaEntityStack.top());
//     javaEntityStack.pop();
//     return javaMethod;
// }


// JavaType JavaEntityVisitor::createJavaType(const IdentifierAsType* typeNode) {
//     typeNode->acceptVisitor(this);
//     auto javaType = std::any_cast<JavaType>(javaEntityStack.top());
//     javaEntityStack.pop();
//     return javaType;
// }


// JavaType JavaEntityVisitor::createJavaType(const ArraySignature* typeNode) {
//     typeNode->acceptVisitor(this);
//     auto javaType = std::any_cast<JavaType>(javaEntityStack.top());
//     javaEntityStack.pop();
//     return javaType;
// }


// JavaClass JavaEntityVisitor::createJavaClass(const StructSignature* structSignatureNode) {
//     structSignatureNode->acceptVisitor(this);
//     auto javaClass = std::any_cast<JavaClass>(javaEntityStack.top());
//     javaEntityStack.pop();
//     return javaClass;
// }

// JavaClass JavaEntityVisitor::createJavaClass(const InterfaceType* structSignatureNode) {
//     structSignatureNode->acceptVisitor(this);
//     auto javaClass = std::any_cast<JavaClass>(javaEntityStack.top());
//     javaEntityStack.pop();
//     return javaClass;
// }
*/