#include "class_entity.h"

TypePtr MethodEntity::toTypeEntity() const {
    std::list<TypePtr> args;

    for (const auto &arg : this->arguments ) {
       args.push_back(arg.second);
    }

    return std::make_shared<const TypeEntity>(FunctionSignatureEntity(std::move(args), this->returnType));
}


MethodEntity::MethodEntity(FunctionDeclaration* node) : block(node->block.get()) {
    // fill with args
    for (const auto& identifiersWithType : node->signature->idsAndTypesArgs) {
        auto type = std::make_shared<const TypeEntity>(identifiersWithType->type.get());
        
        for (const auto& identifier : identifiersWithType->identifiers) {
            arguments.emplace_back(identifier, type);
        }
    }

    if (node->signature->idsAndTypesResults.empty()) {
        returnType = std::make_shared<const TypeEntity>(TypeEntity::Void);
    } else {
        // fill with return values
        for (const auto& identifiersWithType : node->signature->idsAndTypesResults) {
            auto type = std::make_shared<const TypeEntity>(identifiersWithType->type.get());
            returnType = type;
        }
    }
 }

bool ClassEntity::addFields(std::unordered_map<std::string, std::unique_ptr<FieldEntity>> vars) {
    bool success = true;
    for (auto& [identifier, field] : vars) {
        success &= addField(identifier, std::move(field));
    }
    return success;
}


void MethodEntity::setNumberLocalVariables(int number) {
    numberLocalVariables = number;
}

int MethodEntity::getNumberLocalVariables() const {
    return numberLocalVariables;
}
