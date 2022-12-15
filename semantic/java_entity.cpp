#include "java_entity.h"

JavaType::JavaType(std::string typeAsId) {    
    // TODO typeAsId == "uint8" || typeAsId == "uint16" || typeAsId == "uint32" || typeAsId == "uint64"

    if (typeAsId == "int"   || typeAsId == "int16"  || typeAsId == "int32"  || typeAsId == "int64" || typeAsId == "rune")
        type = BuiltInType::Int;

    else if (typeAsId == "float32" || typeAsId == "float64")
        type = BuiltInType::Float;

    else if (typeAsId == "string")
        type = BuiltInType::String;

    else if (typeAsId == "bool")
        type = BuiltInType::Boolean;

    else 
        type = BuiltInType::UserType;
        
    this->value = typeAsId;
}

JavaType::JavaType(JavaArraySignature array) {
    type = BuiltInType::Array;
    this->value = array;
}

JavaClass::JavaClass(TypeAST *node): typeNode(node) {}

void JavaClass::addMethod(JavaMethod&& method) { methods.push_back(method); }

void JavaClass::addField(JavaVariable&& field) { fields.push_back(field); }

void JavaClass::addField(std::vector<JavaVariable>&& newFields) {
    fields.insert(fields.end(), std::move_iterator(newFields.begin()), std::move_iterator(newFields.end()));
}
