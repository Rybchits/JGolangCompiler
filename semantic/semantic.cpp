#include <iostream>
#include "semantic.h"

bool Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return;
    }

    analyzePackageScope();
}

void Semantic::analyzePackageScope() {
    std::vector<MethodDeclaration> methods;

    // Collect gloabal type
    for (auto decl : *root->topDeclarations) {
        auto casted = As<TypeDeclaration>(decl);

        if (decl == nullptr)
            continue;

        globalAliases.insert(casted->alias, casted->declType);
    }
}


