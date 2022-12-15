#include "semantic.h"
#include "classes_node_visitor.h"

#include <iostream>

bool Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return false;
    }
    findAnonymousClass();
    analyzePackageScope();
    return true;
}

const std::string Semantic::GlobalClassName = "$GLOBAL";

bool Semantic::isGeneratedName(const std::string_view name) { return !name.empty() && name[0] == '$'; };

void Semantic::findAnonymousClass() {
    auto visitor = new ClassesNodeVisitor();
    root->acceptVisitor(visitor, TraversalMethod::Upward);
    classes.insert(visitor->classes.begin(), visitor->classes.end());
}

void Semantic::analyzePackageScope() {
    std::vector<MethodDeclaration*> methods;

    for (auto decl : *root->topDeclarations) {

        if (auto typeDeclaration = dynamic_cast<TypeDeclaration*>(decl)) {
            classes[typeDeclaration->alias] = JavaClass(typeDeclaration->declType);

        } else if (auto methodDeclaration = dynamic_cast<MethodDeclaration*>(decl)) {
            methods.push_back(methodDeclaration);

        } else if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl)) {
            classes[GlobalClassName].addMethod(functionDeclaration);
        }
    }

    for (auto method : methods) {
        if (auto className = dynamic_cast<IdentifierAsType*>(method->receiverType)) {
            if (classes.find(className->identifier) != classes.end()) {
                classes[className->identifier].addMethod(JavaMethod(method));
            } else {
                errors.push_back("Unknown receiver type at method " + method->identifier + ": " + className->identifier);
            }
        } else {
            errors.push_back("Invalid receiver type at method " + method->identifier);
        }
    }
}

// 1) Собрать все классы
// StructSignature -> JavaClass
// MethodSignature -> JavaMethod