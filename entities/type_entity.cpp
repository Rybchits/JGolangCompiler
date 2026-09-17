#include "type_entity.h"
#include "../ast.h"

bool ArraySignatureEntity::equal(const ArraySignatureEntity& other) const {
    return this->elementType->equal(other.elementType) && this->dims == other.dims;
}

bool ArraySignatureEntity::isSlice() const {
    return dims == -1;
}

bool FunctionSignatureEntity::equals(const FunctionSignatureEntity& other) const {
    if (this->argsTypes.size() != other.argsTypes.size()) {
        return false;
    }

    bool argsEquals = false;
    std::list<TypePtr>::const_iterator it1 = this->argsTypes.begin();
    std::list<TypePtr>::const_iterator it2 = other.argsTypes.begin();

    while(it1 != this->argsTypes.end() && it2 != other.argsTypes.end() && argsEquals) {
        argsEquals &= (*it1)->equal((*it2));
        it1++;
        it2++;
    }

    return argsEquals && this->returnType->equal(other.returnType);
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

bool TypeEntity::isInteger() const {
    return this->type == TypeEntityEnum::Int || this->type == TypeEntityEnum::UntypedInt;
}

bool TypeEntity::isFloat() const {
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

bool TypeEntity::equal(const TypePtr& other) const {
    
    if (this->type == Array && other->type == Array) {
        const auto& currentValue = std::get<ArraySignatureEntity>(this->value);
        const auto& otherValue = std::get<ArraySignatureEntity>(other->value);
        return currentValue.equal(otherValue);
        
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

TypePtr determinePriorityType(const TypePtr& lhs, const TypePtr& rhs) {
    if (!lhs->equal(rhs)) {
        return std::make_shared<const TypeEntity>();
    }

    const auto left = lhs->type;
    const auto right = rhs->type;
    if ((left == TypeEntity::UntypedInt && right == TypeEntity::UntypedFloat)
        || (left == TypeEntity::UntypedFloat && right == TypeEntity::UntypedInt)) {
        return std::make_shared<const TypeEntity>(TypeEntity::UntypedFloat);
    }
    if ((left == TypeEntity::Int && right == TypeEntity::UntypedInt)
        || (left == TypeEntity::UntypedInt && right == TypeEntity::Int)) {
        return std::make_shared<const TypeEntity>(TypeEntity::Int);
    }
    if ((left == TypeEntity::UntypedFloat && right == TypeEntity::Float)
        || (left == TypeEntity::Float && right == TypeEntity::UntypedFloat)
        || (left == TypeEntity::UntypedInt && right == TypeEntity::Float)
        || (left == TypeEntity::Float && right == TypeEntity::UntypedInt)) {
        return std::make_shared<const TypeEntity>(TypeEntity::Float);
    }
    return lhs;
}

TypeEntity::TypeEntity(const TypeAST* node) {
    if (auto array = dynamic_cast<const ArraySignature*>(node)) {
        type = TypeEntityEnum::Array;
        this->value = ArraySignatureEntity(array->dimensions, std::make_shared<const TypeEntity>(array->arrayElementType));

    } else if (auto typeAsId = dynamic_cast<const IdentifierAsType*>(node)) {
        this->type = builtInTypeFromString(typeAsId->identifier);
        this->value = typeAsId->identifier;
        
    } else {
        this->type = TypeEntityEnum::Invalid;
    }
};

bool TypeEntity::isNumeric() const {
    return type == Int || type == Float || type == UntypedInt || type == UntypedFloat;
}

std::string TypeEntity::toByteCode() const {
    if (type == Array)
        return "[" + std::get<ArraySignatureEntity>(value).elementType->toByteCode();
    
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

    else if (type == Any)
        return "Ljava/lang/Object;";
    
    else if (type == UserType)
        return std::get<std::string>(value);

    else if (type == BuiltInFunction)
        return "built-in: " + std::get<std::string>(value);

    else if (type == Function) {
        std::string code = "(";

        const auto& func = std::get<FunctionSignatureEntity>(value);
        for (auto arg : func.argsTypes) {
            code += arg->toByteCode();
        }

        code += ")";
        code += func.returnType->toByteCode();

        return code;
    }
        
    return "Invalid";
}


TypePtr typeAxis(TypePtr type, int indexAxis) {
    if (indexAxis < 0) {
        return std::make_shared<const TypeEntity>();
    }
    while (indexAxis-- > 0) {
        const auto* array = std::get_if<ArraySignatureEntity>(&type->value);
        if (!array) {
            return std::make_shared<const TypeEntity>();
        }
        type = array->elementType;
    }
    return type;
}
