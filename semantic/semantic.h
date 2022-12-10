#pragma once
#include <vector>
#include <unordered_map>

#include "./java_entity.h"
#include "../ast.h"

class Semantic {
private:
    PackageAST* root;

    std::unordered_map<std::string, TypeAST> globalAliases;     /* Строится из TypeDeclarations. key = string (id), value = TypeAST (сам тип, содержимое)
                                                                   Раскрываем элиасы ==> 



                                                                   if (dynamic_cast<IdentifierAsType*>(value) != nullptr) {     //TODO захватить структуры
                                                                        
                                                                        if (dynamic_cast<IdentifierAsType*>(TypeAST) == базовый тип) {
                                                                            true_type = dynamic_cast<IdentifierAsType*>(TypeAST)->identifier;
                                                                            value = globalAliases[true_type];
                                                                        }
                                                                        else {
                                                                            globalAliases[true_type].*выполнить ту же самую проверку, пока не дойдем до базового типа*
                                                                        }
                                                                   }

                                                                */
    // Declared global
    std::vector<JavaClass> classes;
    std::vector<JavaMethod> functions;
    std::vector<JavaVariable> globals;

    void analyzePackageScope();

public:
    explicit Semantic(PackageAST* package): root(package) {};

    bool analyze();
    std::vector<std::string> errors;
};
