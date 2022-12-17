#include "semantic.h"
#include "transformation_visitor.h"

bool Semantic::analyze() {
    if (root == nullptr) {
        errors.emplace_back("Root node is empty");
        return false;
    }
    transformRoot();
    analyzePackageScope();
    return true;
}

void Semantic::transformRoot() {
    auto visitor = new TreeTransformationVisitor(this);
    std::unordered_map<std::string, JavaClass> classesFromTransform = visitor->transform(root);
    classes.insert(classesFromTransform.begin(), classesFromTransform.end());
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

Semantic* Semantic::instance = nullptr;

Semantic *Semantic::GetInstance(PackageAST *package) {
    if (instance == nullptr) {
        instance = new Semantic(package);
    }
    return instance;
}

const std::string Semantic::GlobalClassName = "$GLOBAL";

bool Semantic::isGeneratedName(const std::string_view name) { return !name.empty() && name[0] == '$'; };
