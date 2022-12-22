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

    return true;
}

void Semantic::transformRoot() {
    auto loopVisitor = new LoopsVisitor(this);
    loopVisitor->transform(root);
}

void Semantic::analyzeTypesAndVariables() {
    auto typeVisitor = new TypeCheckVisitor(this);
    typeVisitor->check(root);
}

void Semantic::analyzePackageScope() {
    for (auto decl : root->topDeclarations) {
        if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl)) {
            functions[functionDeclaration->identifier] = JavaFunction(functionDeclaration);
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
