#include "ast_utils.h"

#include <memory>
#include <utility>

TypeList ListIdentifiersToListTypes(const IdentifiersList& identifiers) {
    TypeList types;
    for (const auto& id : identifiers) {
        types.push_back(std::make_unique<IdentifierAsType>(id));
    }
    return types;
}

IdentifiersWithTypeList AttachIdentifiersToListTypes(TypeList listTypes) {
    IdentifiersWithTypeList result;
    for (auto& type : listTypes) {
        result.push_back(std::make_unique<IdentifiersWithType>("_", std::move(type)));
    }
    return result;
}

std::optional<IdentifiersList> IdentifiersListFromExpressions(const ExpressionList& expressions) {
    IdentifiersList identifiers;
    for (const auto& expression : expressions) {
        auto* identifier = dynamic_cast<IdentifierAsExpression*>(expression.get());
        if (!identifier) return std::nullopt;
        identifiers.push_back(identifier->identifier);
    }
    return identifiers;
}
