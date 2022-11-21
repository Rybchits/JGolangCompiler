#pragma once

#include "tree_nodes.h"

/* -------------------------------- Types -------------------------------- */
std::list<std::string> BuiltInTypes = {
        "int",
        "int8",
        "int16",
        "int32",
        "int64",
        "uint8",
        "uint16",
        "uint32",
        "uint64",
        "float32",
        "float64",
        "string",
        "rune",
        "bool"
};

bool IdentifierAsType::isBuiltInType() {
    return std::find(BuiltInTypes.begin(), BuiltInTypes.end(), identifier) != BuiltInTypes.end();
}


std::list<IdentifierAsType *> IdentifierAsType::ListIdentifiersToListTypes(IdentifiersList& identifiers) {
    auto listTypes = new std::list<IdentifierAsType *>;

    if (identifiers.empty()) { return *listTypes; }

    for (auto id: identifiers) {
        listTypes->push_back(new IdentifierAsType(id));
    }

    return *listTypes;
}


std::list<IdentifiersWithType *> *IdentifiersWithType::AttachIdentifiersToListTypes(std::list<TypeAST *>& listTypes) {
    auto listIdentifiersWithType = new std::list<IdentifiersWithType *>;

    if (listTypes.empty()) { return listIdentifiersWithType; }

    for (auto type: listTypes) {
        auto ids = new IdentifiersList();
        ids->push_back("_");
        listIdentifiersWithType->push_back(new IdentifiersWithType(*ids, type));
    }

    return listIdentifiersWithType;
}


