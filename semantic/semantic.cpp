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
        printErrors();
        return false;
    }

    analyzePackageScope();

    if (!errors.empty()) {
        printErrors();
        return false;
    }
    
    createGlobalClass();

    if (!errors.empty()) {
        printErrors();
        return false;
    }

    return true;
}

void Semantic::transformRoot() {
    auto loopVisitor = new LoopsVisitor(this);
    loopVisitor->transform(root);
}

void Semantic::createGlobalClass() {
    auto typeVisitor = new TypeCheckVisitor(this);
    packageClass = typeVisitor->createGlobalClass(packageFunctions, packageVariables);
}

void Semantic::analyzePackageScope() {
    bool findMain = false;

    // Collect all functions
    for (auto decl : root->topDeclarations) {
        if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl)) {
            if (functionDeclaration->identifier == "main") {
                if (!functionDeclaration->signature->idsAndTypesArgs.empty() 
                        || !functionDeclaration->signature->idsAndTypesResults.empty()) {

                            errors.push_back("Function main must have no arguments and no return values");
                }

                functionDeclaration->signature->idsAndTypesArgs.push_back(
                    new IdentifiersWithType(*(new IdentifiersList({"args"})), new ArraySignature(new IdentifierAsType("string")))  
                );
                findMain = true;
            }

            packageFunctions.push_back(functionDeclaration);
            
        } else if (auto variableDeclaration = dynamic_cast<VariableDeclaration*>(decl)) {
            packageVariables.push_back(variableDeclaration);
        }
    }

    if (!findMain) {
        errors.push_back("Does not contain the 'main' function");
    }
}

Semantic* Semantic::instance = nullptr;

Semantic *Semantic::GetInstance(PackageAST *package) {
    if (instance == nullptr) {
        instance = new Semantic(package);
    }
    return instance;
}

void Semantic::printErrors() {
    for (auto err : errors) {
        std::cout << "Error: " << err << std::endl;
    }
}

const std::string Semantic::GlobalClassName = "$GLOBAL";

bool Semantic::isGeneratedName(const std::string_view name) { return !name.empty() && name[0] == '$'; };
