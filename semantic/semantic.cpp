#include "semantic.h"
#include "visitors/loops_visitor.h"
#include "visitors/type_check_visitor.h"

#include <iostream>

bool Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return false;
    }

    transformRoot();

    if (!errors.empty()) {
        for (auto err : errors) {
            std::cout << err << std::endl;
        }
        return false;
    }

    analyzePackageScope();

    analyzeTypesAndVariables();

    if (!errors.empty()) {
        for (auto err : errors) {
            std::cout << err << std::endl;
        }
        return false;
    }

    return true;
}

void Semantic::transformRoot() {
    auto loopVisitor = new LoopsVisitor(this);
    loopVisitor->transform(root);
}

void Semantic::analyzeTypesAndVariables() {
    auto typeVisitor = new TypeCheckVisitor(this);
    typeVisitor->checkGlobalClass(classes[Semantic::GlobalClassName], packageVariables);
}

void Semantic::analyzePackageScope() {
    // Collect all functions
    for (auto decl : root->topDeclarations) {
        if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl)) {
            
            if (!classes[GlobalClassName]->addMethod(functionDeclaration->identifier, new JavaFunction(functionDeclaration))) {
                errors.push_back(functionDeclaration->identifier + " redeclared in this block"); 
            }
        } else if (auto variableDeclaration = dynamic_cast<VariableDeclaration*>(decl)) {
            packageVariables.push_back(variableDeclaration);
        }
    }
}

Semantic* Semantic::instance = nullptr;

Semantic *Semantic::GetInstance(PackageAST *package) {
    if (instance == nullptr) {
        instance = new Semantic(package);
    }
    return instance;
}

const std::string Semantic::GlobalClassName = "$GLOBAL";

bool Semantic::isGeneratedName(const std::string_view name) { return !name.empty() && name[0] == '$'; };
