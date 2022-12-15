// #pragma once

// #include "../visitor.h"
// #include "./java_entity.h"

// #include <stack>
// #include <any>
// #include <utility>

// class JavaEntityVisitor : public Visitor {
// private:
//     std::stack<std::any> javaEntityStack;

//     void visit(const NodeAST* node) override {};
//     void visit(const IdentifiersWithType* node) override;
//     void visit(const IdentifierAsType* node) override;
//     void visit(const StructSignature* node) override;
//     void visit(const ArraySignature* node) override;
//     void visit(const MethodDeclaration* node) override;
//     void visit(BlockStatement* node) override;
//     void visit(FunctionSignature* node) override;

// public:
//     JavaType createJavaType(const IdentifierAsType* typeNode);
//     JavaType createJavaType(const ArraySignature* arraynode);

//     std::vector<JavaVariable> createJavaVariable(const IdentifiersWithType* typedIdsNode);
//     JavaMethod createJavaMethod(const FunctionDeclaration* functionNode);

//     JavaClass createJavaClass(const StructSignature* structSignatureNode);
//     JavaClass createJavaClass(const InterfaceType* interfaceNode);
// };
