#include "java_entity.h"

JavaType::BuiltInType JavaType::builtInTypeFromString(std::string id) {

    if (id == "int"   || id == "int16"  || id == "int32"  || id == "int64" || id == "rune")
        return BuiltInType::Int;

    else if (id == "float32" || id == "float64")
        return BuiltInType::Float;

    else if (id == "string")
        return BuiltInType::String;

    else if (id == "bool")
        return BuiltInType::Boolean;

    else 
        return BuiltInType::UserType;
}

JavaType::JavaType(TypeAST* node) {
    if (auto array = dynamic_cast<ArraySignature*>(node)) {
        type = BuiltInType::Array;
        this->value = JavaArraySignature(array->dimensions, new JavaType(array->arrayElementType));

    } else if (auto typeAsId = dynamic_cast<IdentifierAsType*>(node)) {
        this->type = builtInTypeFromString(typeAsId->identifier);
        this->value = typeAsId->identifier;
    }
};

JavaType::JavaType(std::string id) {
    this->type = builtInTypeFromString(id);
    this->value = id;
}

bool JavaType::operator==(const JavaType& other) {
    if (this->type == Unknown || other.type == Unknown) {
        return false;
    } else if (this->type == Array && other.type == Array) {
        // TODO HERE
        //std::get<>()
        //return this->
    }

    return false;
}

std::string JavaType::toByteCode() const {
    switch (type)
    {
    case BuiltInType::Array:
        
        break;
    
    default:
        break;
    }

    return "";
}
