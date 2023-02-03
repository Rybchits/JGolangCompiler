#include "class_entity.h"

TypeEntity* MethodEntity::toTypeEntity() {
    std::list<TypeEntity*> args;

    for (const auto &arg : this->arguments ) {
       args.push_back(arg.second);
    }

    return new TypeEntity(new FunctionSignatureEntity(args, this->returnType));
}


MethodEntity::MethodEntity(FunctionDeclaration* node) : block(node->block) {
    // fill with args
    for (auto identifiersWithType : node->signature->idsAndTypesArgs) {
        auto type = new TypeEntity(identifiersWithType->type);
        
        for (auto identifier : identifiersWithType->identifiers) {
            arguments.emplace_back(identifier, type);
        }
    }

    if (node->signature->idsAndTypesResults.empty()) {
        returnType = new TypeEntity(TypeEntity::Void);
    } else {
        // fill with return values
        for (auto identifiersWithType : node->signature->idsAndTypesResults) {
            auto type = new TypeEntity(identifiersWithType->type);
            returnType = type;
        }
    }
 }

 bool ClassEntity::addFields(std::unordered_map<std::string, FieldEntity*> & vars) {
        bool success = true;

        for (auto & [identifier, type] : vars) {
            success &= fields.try_emplace(identifier, type).second;
        }

        return success;
    };


void MethodEntity::setNumberLocalVariables(int number) {
    numberLocalVariables = number;
}

int MethodEntity::getNumberLocalVariables() const {
    return numberLocalVariables;
}
