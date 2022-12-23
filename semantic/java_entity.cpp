#include "java_entity.h"

bool JavaArraySignature::equals(const JavaArraySignature* other) {
    return this->dims == other->dims && this->type == other->type;
}

JavaType::JavaTypeEnum JavaType::builtInTypeFromString(std::string id) {

    if (id == "int"   || id == "int16"  || id == "int32"  || id == "int64" || id == "rune")
        return JavaTypeEnum::Int;

    else if (id == "float32" || id == "float64")
        return JavaTypeEnum::Float;

    else if (id == "string")
        return JavaTypeEnum::String;

    else if (id == "bool")
        return JavaTypeEnum::Boolean;

    else
        return JavaTypeEnum::Invalid;
}

bool JavaType::equal(const JavaType* other) {
    if (this->type == Array && other->type == Array) {
        auto currentValue = std::get<JavaArraySignature*>(this->value);
        auto otherValue = std::get<JavaArraySignature*>(other->value);
        return currentValue->equals(otherValue);
        
    } else if (this->type == UserType && other->type == UserType) {
        auto currentValue = std::get<std::string>(this->value);
        auto otherValue = std::get<std::string>(other->value);
        return currentValue == otherValue;
        
    } else if ((type == UntypedInt && other->type == UntypedFloat)
               || (type == UntypedFloat && other->type == UntypedInt)
               || (type == Int && other->type == UntypedInt)
               || (type == UntypedInt && other->type == Int)
               || (type == UntypedFloat && other->type == Float)
               || (type == Float || other->type == UntypedFloat)) {
        return true;
        
    } else if (type == other->type) {
        return true;
    }

    return false;
}

// Определение рузультирующего типа
JavaType* JavaType::determinePriorityType(const JavaType* other) {
    if (this->equal(other)) {
        if ((type == UntypedInt && other->type == UntypedFloat) || (type == UntypedFloat && other->type == UntypedInt)) {
            return new JavaType(UntypedFloat);
            
        } else if ((type == Int && other->type == UntypedInt) || (type == UntypedInt && other->type == Int)) {
            return new JavaType(Int);
            
        } else if ((type == UntypedFloat && other->type == Float) || (type == Float && other->type == UntypedFloat)) {
            return new JavaType(Float);
            
        } else {
            return this;
        }
    } else {
        return nullptr;
    }
}

JavaType::JavaType(TypeAST* node) {
    if (auto array = dynamic_cast<ArraySignature*>(node)) {
        type = JavaTypeEnum::Array;
        this->value = new JavaArraySignature(array->dimensions, new JavaType(array->arrayElementType));

    } else if (auto typeAsId = dynamic_cast<IdentifierAsType*>(node)) {
        this->type = builtInTypeFromString(typeAsId->identifier);
        this->value = typeAsId->identifier;
    }
};

bool JavaType::isNumeric() {
    return type == Int || type == Float || type == UntypedInt || type == UntypedFloat;
}

std::string JavaType::toByteCode() const {
    if (type == Array)
        return "[" + std::get<JavaArraySignature*>(value)->type->toByteCode();
    
    else if (type == Int || type == UntypedInt)
        return "I";
    
    else if (type == Float || type == UntypedFloat)
        return "F";
    
    else if (type == Boolean)
        return "Z";
    
    else if (type == String)
        return "java/lang/String";
    
    else if (type == UserType)
        return std::get<std::string>(value);
        
    return "";
}

 JavaFunction::JavaFunction(FunctionDeclaration* node) : block(node->block) {
    // fill with args
    for (auto identifiersWithType : node->signature->idsAndTypesArgs) {
        auto type = new JavaType(identifiersWithType->type);
        for (auto identifier : identifiersWithType->identifiers) {
            arguments.emplace(identifier, type);
        }
    }

    // fill with return values
    for (auto identifiersWithType : node->signature->idsAndTypesResults) {
        auto type = new JavaType(identifiersWithType->type);
        returnType = type;
        for (auto identifier : identifiersWithType->identifiers) {
            // returnType
        }
    }
 }
