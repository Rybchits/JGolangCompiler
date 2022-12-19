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

JavaClass::JavaClass(TypeAST *node): typeNode(node) {
    if (auto structType = dynamic_cast<StructSignature*>(node)) {
        typeJavaClass = JavaClass::TypeJavaClass::Class;
        
    } else if (auto arrayType = dynamic_cast<ArraySignature*>(node)) {
        typeJavaClass = JavaClass::TypeJavaClass::Alias;

    } else if (auto identifierType = dynamic_cast<IdentifierAsType*>(node)) {
        typeJavaClass = JavaClass::TypeJavaClass::Alias;

    } else if (auto interfaceType = dynamic_cast<InterfaceType*>(node)) {
        typeJavaClass = JavaClass::TypeJavaClass::Interface;
    } 
}

void JavaClass::addMethod(JavaMethod&& method) { methods.push_back(method); }

void JavaClass::addField(JavaVariable&& field) { fields.push_back(std::move(field)); }

void JavaClass::addField(const JavaVariable& field) { fields.push_back(field); }

void JavaClass::addField(std::vector<JavaVariable>&& newFields) {
    fields.insert(fields.end(), std::move_iterator(newFields.begin()), std::move_iterator(newFields.end()));
}

void JavaClass::addField(const std::vector<JavaVariable>& newFields) {
    fields.insert(fields.end(), newFields.begin(), newFields.end());
}

void JavaClass::constructFields() {
    if (auto structSignature = dynamic_cast<StructSignature *>(typeNode)) {
        for (auto& field : structSignature->structMembers) {
            if (field->identifiers.size() == 1 && field->identifiers.front() == "") {
                if (auto compositionType = dynamic_cast<IdentifierAsType *>(field->type) /* TODO check type existing in type scope */) {
                    //TODO add composition
                } else {
                    // TODO exception 'syntax error: unexpected struct, expecting field name or embedded type'
                }
            }
            for (auto& identifier : field->identifiers) {
                if (auto identifierAsType = dynamic_cast<IdentifierAsType *>(field->type)) {
                    addField(JavaVariable(identifier, new JavaType(identifierAsType->identifier))); // TODO можно не порождать для списка id постоянно тип, т.к. у них он один и тот же
                } else if (auto arraySignatureType = dynamic_cast<ArraySignature *>(field->type)) {
                    // TODO как-то хитро сделать
                } else if (auto intefraceType = dynamic_cast<InterfaceType* >(field->type)) {
                    // TODO interface type
                }
            }
        }
    } else if (auto identifierAsType = dynamic_cast<IdentifierAsType *>(typeNode)) {
        auto identifier = identifierAsType->identifier;
        // addField(classes[identifier].fields);    // предполагается, что поля у этого класса уже собраны
                                                    // Хотя если делаем элиас на элиас, то они могут быть не собраны
                                                    // (при том, что сначала собираем классы, потом элиасы)
    } else if (auto interfaceType = dynamic_cast<InterfaceType *>(typeNode)) {
        // TODO class - interface
    }
}
