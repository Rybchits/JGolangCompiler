#include "semantic.h"
#include <iostream>

bool Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return false;
    }

    analyzePackageScope();
    return true;
}

const std::string Semantic::GlobalClassName = "$GLOBAL";

void Semantic::analyzePackageScope() {
    std::vector<MethodDeclaration*> methods;

    for (auto decl : *root->topDeclarations) {

        if (auto typeDeclaration = dynamic_cast<TypeDeclaration*>(decl)) {
            classes[typeDeclaration->alias] = JavaClass(typeDeclaration->declType);
            //std::cout << "Class " << typeDeclaration->alias << std::endl;

        } else if (auto methodDeclaration = dynamic_cast<MethodDeclaration*>(decl)) {
            methods.push_back(methodDeclaration);
            //std::cout << "Method " << methodDeclaration->identifier << std::endl;

        } else if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl)) {
            classes[GlobalClassName].addMethod(functionDeclaration);
            //std::cout << "Function " << functionDeclaration->identifier << std::endl;
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

    for (const auto& [ key, value ] : classes) {
        
    }

    for () {
        if (auto structType = dynamic_cast<StructSignature*>(typeNode)) {
            typeJavaClass = TypeJavaClass::Class;
            
        } else if (auto arrayType = dynamic_cast<ArraySignature*>(typeNode)) {
            typeJavaClass = TypeJavaClass::Alias;

        } else if (auto identifierType = dynamic_cast<IdentifierAsType*>(typeNode)) {
            typeJavaClass = TypeJavaClass::Alias;

        } else if (auto interfaceType = dynamic_cast<InterfaceType*>(typeNode)) {
            typeJavaClass = TypeJavaClass::Interface;
        }
    }
}

// 1) Собрать все классы
// StructSignature -> JavaClass
// MethodSignature -> JavaMethod