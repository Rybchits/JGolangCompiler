#include <iostream>

#include "tree_nodes.h"

int main() {
    IdentifiersList* ids = new IdentifiersList({"int", "int8"});

    TypeList* types = ListIdentifiersToListTypes(*ids);
    std::list<IdentifiersWithType* > *typedIds = AttachIdentifiersToListTypes(*types);

    for (const auto& i : *ids) {
        std::cout << i << std::endl;
    }
    return 0;
}

/*
 auto oru = new BinaryExpression(BinaryExpression::ExpTypeOr, new BooleanExpression(false),  new BooleanExpression(true));
    std::list<ExpressionAST *> * exps = new std::list<ExpressionAST *>({new BooleanExpression(false), oru});
    ExpressionsList* list = new ExpressionsList();
    list->push_back(new BinaryExpression(BinaryExpression::ExpTypeOr, new BooleanExpression(false),  new BooleanExpression(true)));
    list->push_back(new BooleanExpression(false));

    for (auto el : *exps) {
        std::cout << el->name() << std::endl;
    }
    TypeAST* node3 = new ArraySignature(new ArraySignature(new IdentifierAsType("int")));

    IdentifiersList identifiers;
    identifiers.assign({ "float", "int" });

    std::list<TypeAST *>* a = ListIdentifiersToListTypes(identifiers);
    std::list<TypeAST *> types = { new IdentifierAsType("q") };

    types.assign(a->begin(), a->end());

    types.push_back(new IdentifierAsType("int"));

    auto* typesids = AttachIdentifiersToListTypes(types);

*/