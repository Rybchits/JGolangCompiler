#pragma once

#include "../ast.h"

#include <optional>

IdentifiersWithTypeList AttachIdentifiersToListTypes(TypeList listTypes);
TypeList ListIdentifiersToListTypes(const IdentifiersList& identifiers);
std::optional<IdentifiersList> IdentifiersListFromExpressions(const ExpressionList& expressions);
