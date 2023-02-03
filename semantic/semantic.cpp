#include "semantic.h"
#include "./statements_visitor.h"
#include "./precalculate_visitor.h"
#include "./types_visitor.h"

#include <iostream>

Semantic::Semantic(PackageAST* package): root(package), typeVisitor(new TypesVisitor(this)) {}

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
    auto visitor = new StatementsVisitor(this);
    visitor->transform(root);
}

void Semantic::precalculateExpressions() {
    auto visitor = new PrecalculateVisitor(this);
    visitor->transform(root);
}

void Semantic::createPackageClass() {
    packageClass = new ClassEntity();
    auto idsConstants = std::vector<std::string>();

    // Add package functions
    for (auto function : packageFunctions) {
        auto method =  new MethodEntity(function);

        if (!packageClass->addMethod(function->identifier, method)) {
            addError(function->identifier + "redclared in block");
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
                expressionNode = (*expressionsIter);
                expressionsIter++;
            }
            
            FieldEntity* field;

            // Blank static variables can't be deleted. We need to make them unique and unused
            if (identifier == "_") {
                identifier = "$_" + std::to_string(indexBlankVariable);
                indexBlankVariable++;
                field = new FieldEntity(new TypeEntity(TypeEntity::Any), expressionNode);

            } else {
                field = new FieldEntity(new TypeEntity(variable->identifiersWithType->type), expressionNode);
            }

            if (variable->isConst)
                idsConstants.push_back(identifier);

            if (!packageClass->addField(identifier, field)) {
                addError(identifier + "already redclared in package");
            }
        }
    }

    typeVisitor->analyzePackageClass(packageClass, idsConstants);
}

void Semantic::analyzePackageScope() {
    bool findMain = false;

    for (auto decl : root->topDeclarations) {

        // add method package class
        if (auto functionDeclaration = dynamic_cast<FunctionDeclaration*>(decl)) {
            if (functionDeclaration->identifier == "main") {

                if (!functionDeclaration->signature->idsAndTypesArgs.empty() || !functionDeclaration->signature->idsAndTypesResults.empty()) {
                    addError("Function main must have no arguments and no return values");
                }

                functionDeclaration->signature->idsAndTypesArgs.push_back(
                    new IdentifiersWithType(*(new IdentifiersList({"$args"})), new ArraySignature(new IdentifierAsType("string")))  
                );
                
                findMain = true;
            }

            packageFunctions.push_back(functionDeclaration);
        
        // add field package class
        } else if (auto variableDeclaration = dynamic_cast<VariableDeclaration*>(decl)) {
            packageVariables.push_back(variableDeclaration);
        }
    }

    if (!findMain) {
        addError("Does not contain the 'main' function");
    }
}

TypesVisitor* Semantic::getTypesVisitor() {
    return typeVisitor; 
}

void Semantic::addError(std::string message) {
    errors.push_back(message);
}

void Semantic::printErrors() {
    for (auto err : errors) {
        std::cout << "Error: " << err << std::endl;
    }
}

const std::vector<std::string> Semantic::BuiltInFunctions = {
    "print", 
    "println", 
    "len", 
    "append", 
    "readInt", 
    "readFloat", 
    "readString", 
    "readBool",
};

bool Semantic::IsBuiltInFunction(std::string identifier) {
    return std::find(BuiltInFunctions.begin(), BuiltInFunctions.end(), identifier) != BuiltInFunctions.end();
}

bool Semantic::isGeneratedName(const std::string_view name) { return !name.empty() && name[0] == '$'; };
