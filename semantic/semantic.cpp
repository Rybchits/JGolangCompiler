#include "semantic.h"
#include "./statements_visitor.h"
#include "./precalculate_visitor.h"
#include "./types_visitor.h"

#include <iostream>

Semantic::Semantic(PackageAST* package): root(package) {}

bool Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return false;
    }

    transformStatements();

    if (!errors.empty()) {
        printErrors();
        return false;
    }

    precalculateExpressions();
    analyzePackageScope();

    if (!errors.empty()) {
        printErrors();
        return false;
    }
    
    createPackageClass();

    if (!errors.empty()) {
        printErrors();
        return false;
    }

    return true;
}

void Semantic::transformStatements() {
    StatementsVisitor visitor;
    visitor.transform(root);
    const auto& statementErrors = visitor.getErrors();
    errors.insert(errors.end(), statementErrors.begin(), statementErrors.end());
}

void Semantic::precalculateExpressions() {
    auto visitor = PrecalculateVisitor();
    visitor.transform(root);
}

void Semantic::createPackageClass() {
    packageClass = std::make_unique<ClassEntity>();
    auto idsConstants = std::vector<std::string>();

    // Add package functions
    for (const auto& function : packageFunctions) {
        auto method = std::make_unique<MethodEntity>(function);

        if (!packageClass->addMethod(function->identifier, std::move(method))) {
            errors.push_back(function->identifier + "redclared in block");
        }
    }

    // Add package variables
    int indexBlankVariable = 0;
    for (auto it = packageVariables.rbegin(); it != packageVariables.rend(); ++it) {
        auto variable = *it;

        auto expressionsIter = variable->values.begin();
        for (auto identifier : variable->identifiersWithType->identifiers) {

            ExpressionAST* expressionNode = nullptr;
            if (expressionsIter != variable->values.end()) {
                expressionNode = expressionsIter->get();
                expressionsIter++;
            }
            
            std::unique_ptr<FieldEntity> field;

            // Blank static variables can't be deleted. We need to make them unique and unused
            if (identifier == "_") {
                identifier = "$_" + std::to_string(indexBlankVariable);
                indexBlankVariable++;
                field = std::make_unique<FieldEntity>(std::make_shared<const TypeEntity>(TypeEntity::Any), expressionNode);

            } else {
                field = std::make_unique<FieldEntity>(std::make_shared<const TypeEntity>(variable->identifiersWithType->type.get()), expressionNode);
            }

            if (variable->isConst)
                idsConstants.push_back(identifier);

            if (!packageClass->addField(identifier, std::move(field))) {
                errors.push_back(identifier + "already redclared in package");
            }
        }
    }

    TypesVisitor typeVisitor;
    typeVisitor.analyzePackageClass(packageClass.get(), idsConstants);
    const auto& typeErrors = typeVisitor.getErrors();
    errors.insert(errors.end(), typeErrors.begin(), typeErrors.end());
}

void Semantic::analyzePackageScope() {
    bool findMain = false;

    for (const auto& decl : root->topDeclarations) {

        // add method package class
        if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl.get())) {
            if (functionDeclaration->identifier == "main") {

                if (!functionDeclaration->signature->idsAndTypesArgs.empty() || !functionDeclaration->signature->idsAndTypesResults.empty()) {
                    errors.push_back("Function main must have no arguments and no return values");
                }

                functionDeclaration->signature->idsAndTypesArgs.push_back(
                    std::make_unique<IdentifiersWithType>(IdentifiersList({"$args"}), std::make_unique<ArraySignature>(std::make_unique<IdentifierAsType>("string")))
                );
                
                findMain = true;
            }

            packageFunctions.push_back(functionDeclaration);
        
        // add field package class
        } else if (auto variableDeclaration = dynamic_cast<VariableDeclaration*>(decl.get())) {
            packageVariables.push_back(variableDeclaration);
        }
    }

    if (!findMain) {
        errors.push_back("Does not contain the 'main' function");
    }
}

void Semantic::printErrors() {
    for (const auto& err : errors) {
        std::cout << "Error: " << err << std::endl;
    }
}

bool Semantic::isGeneratedName(const std::string_view name) { return !name.empty() && name[0] == '$'; };
