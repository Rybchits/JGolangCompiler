#include "entities.h"

bool ArraySignatureEntity::equals(const ArraySignatureEntity* other) const {
    return this->elementType->equal(other->elementType) && this->dims == other->dims;
}

bool ArraySignatureEntity::isSlice() const {
    return dims == -1;
}

bool FunctionSignatureEntity::equals(const FunctionSignatureEntity* other) const {
    if (this->argsTypes.size() != other->argsTypes.size()) {
        return false;
    }

    bool argsEquals = false;
    std::list<TypeEntity*>::const_iterator it1 = this->argsTypes.begin();
    std::list<TypeEntity*>::const_iterator it2 = other->argsTypes.begin();

    while(it1 != this->argsTypes.end() && it2 != other->argsTypes.end() && argsEquals) {
        argsEquals &= (*it1)->equal((*it2));
        it1++;
        it2++;
    }

    return argsEquals && this->returnType->equal(other->returnType);
}


std::list<std::string> TypeEntity::BuiltInTypes = {
        "int",
        "int8",
        "int16",
        "int32",
        "int64",
        "float32",
        "float64",
        "string",
        "rune",
        "bool"
};


bool TypeEntity::IsBuiltInType(std::string identifier) {
    return std::find(BuiltInTypes.begin(), BuiltInTypes.end(), identifier) != BuiltInTypes.end();
}

bool TypeEntity::isInteger() {
    return this->type == TypeEntityEnum::Int || this->type == TypeEntityEnum::UntypedInt;
}

bool TypeEntity::isFloat() {
    return this->type == TypeEntityEnum::Float || this->type == TypeEntityEnum::UntypedFloat;
}

TypeEntity::TypeEntityEnum TypeEntity::builtInTypeFromString(std::string id) {

    if (id == "int8" || id == "int" || id == "int16"  || id == "int32"  || id == "int64" || id == "rune")
        return TypeEntityEnum::Int;

    else if (id == "float32" || id == "float64")
        return TypeEntityEnum::Float;

    else if (id == "string")
        return TypeEntityEnum::String;

    else if (id == "bool")
        return TypeEntityEnum::Boolean;

    else
        return TypeEntityEnum::Invalid;
}

bool TypeEntity::equal(const TypeEntity* other) {
    
    if (this->type == Array && other->type == Array) {
        auto currentValue = std::get<ArraySignatureEntity*>(this->value);
        auto otherValue = std::get<ArraySignatureEntity*>(other->value);
        return currentValue->equals(otherValue);
        
    } else if (this->type == UserType && other->type == UserType) {
        auto currentValue = std::get<std::string>(this->value);
        auto otherValue = std::get<std::string>(other->value);
        return currentValue == otherValue;
        
    } else if ((this->type == UntypedInt && other->type == UntypedFloat)
               || (this->type == UntypedFloat && other->type == UntypedInt)
               
               || (this->type == Int && other->type == UntypedInt)
               || (this->type == UntypedInt && other->type == Int)

               || (this->type == UntypedFloat && other->type == Float)
               || (this->type == Float && other->type == UntypedFloat)

               || (this->type == Float && other->type == UntypedInt)
               || (this->type == UntypedInt && other->type == Float)

               || (this->type == Any || other->type == Any)
               || (this->type == other->type)) {
                
                return true;
    }

    return false;
}

// Определение рузультирующего типа
TypeEntity* TypeEntity::determinePriorityType(const TypeEntity* other) {

    if (this->equal(other)) {
        if ((type == UntypedInt && other->type == UntypedFloat) || (type == UntypedFloat && other->type == UntypedInt)) {
            return new TypeEntity(UntypedFloat);
            
        } else if ((type == Int && other->type == UntypedInt) || (type == UntypedInt && other->type == Int)) {
            return new TypeEntity(Int);
            
        } else if ((type == UntypedFloat && other->type == Float) || (type == Float && other->type == UntypedFloat)) {
            return new TypeEntity(Float);
            
        } else if ((type == UntypedInt && other->type == Float) || (type == Float && other->type == UntypedInt)) {
            return new TypeEntity(Float);
            
        } else {
            return this;
        }
    } else {
        return new TypeEntity();
    }
}

TypeEntity::TypeEntity(TypeAST* node) {
    if (auto array = dynamic_cast<ArraySignature*>(node)) {
        type = TypeEntityEnum::Array;
        this->value = new ArraySignatureEntity(array->dimensions, new TypeEntity(array->arrayElementType));

    } else if (auto typeAsId = dynamic_cast<IdentifierAsType*>(node)) {
        this->type = builtInTypeFromString(typeAsId->identifier);
        this->value = typeAsId->identifier;
    }
};

bool TypeEntity::isNumeric() {
    return type == Int || type == Float || type == UntypedInt || type == UntypedFloat;
}

std::string TypeEntity::toByteCode() const {
    if (type == Array)
        return "[" + std::get<ArraySignatureEntity*>(value)->elementType->toByteCode();
    
    else if (type == Int || type == UntypedInt)
        return "I";
    
    else if (type == Float || type == UntypedFloat)
        return "F";
    
    else if (type == Boolean)
        return "Z";

    else if (type == Void)
        return "V";
    
    else if (type == String)
        return "Ljava/lang/String;";
    
    else if (type == UserType)
        return std::get<std::string>(value);

    else if (type == Any)
        return "Ljava/lang/Object;";

    else if (type == Function) {
        std::string code = "(";

        auto func = std::get<FunctionSignatureEntity*>(value);
        for (auto arg : func->argsTypes) {
            code += arg->toByteCode();
        }

        code += ")";
        code += func->returnType->toByteCode();

        return code;
    }
        
    return "Invalid";
}


TypeEntity* ArraySignatureEntity::typeAxis(int indexAxis) {
    auto currentElementType = new TypeEntity(this);

    while (indexAxis != 0) {
        indexAxis--;
        if (std::holds_alternative<ArraySignatureEntity*>(currentElementType->value)) {
            currentElementType = std::get<ArraySignatureEntity*>(currentElementType->value)->elementType;
        } else {
            return new TypeEntity();
        }
    }

    return currentElementType;
}


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
