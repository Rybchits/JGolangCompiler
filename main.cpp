#include <iostream>

#include "tree_nodes.h"

int main() {
    ExpressionsList* list = new ExpressionsList();
    list->push_back(new BinaryExpression(BinaryExpression::ExpTypeOr, new BooleanExpression(false),  new BooleanExpression(true)));
    list->push_back(new BooleanExpression(false));

    for (auto el : *list) {
        std::cout << el->name() << std::endl;
    }
    /*TypeAST* node3 = new ArraySignature(new ArraySignature(new IdentifierAsType("int")));

    IdentifiersList identifiers;
    identifiers.assign({ "float", "int" });

    std::list<IdentifierAsType *> a = IdentifierAsType::ListIdentifiersToListTypes(identifiers);
    std::list<TypeAST *> types;

    types.assign(a.begin(), a.end());

    types.push_back(new IdentifierAsType("int"));

    auto typesids = IdentifiersWithType::AttachIdentifiersToListTypes(types);

    for (auto i : *typesids) {

    }*/
    return 0;
}
