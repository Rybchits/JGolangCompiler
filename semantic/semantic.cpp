#include <iostream>
#include "semantic.h"

void Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return;
    }

    analyzeGlobalClasses();
}

void Semantic::analyzeGlobalClasses() {
    std::vector<MethodDeclaration> methods;

    // пройти по всем верхним объявлениям

        // Если это объявление типа - добавить
        // Если объявление функции
        // объявление или
    for (auto decl : *root->topDeclarations) {
        As<MethodDeclaration>(decl)->block;
    }
}
