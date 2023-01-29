#include "constant.h"

bool operator==(const Constant& lhs, const Constant& rhs)
{
    if (lhs.type != rhs.type) { return false; }
    switch (lhs.type)
    {
        case Constant::TypeT::Utf8:
            return lhs.utf8 == rhs.utf8;
        case Constant::TypeT::Integer:
            return lhs.integer == rhs.integer;
        case Constant::TypeT::Float:
            return lhs.floating == rhs.floating;
        case Constant::TypeT::String:
            return lhs.utf8Id == rhs.utf8Id;
        case Constant::TypeT::Class:
            return lhs.classNameId == rhs.classNameId;
        case Constant::TypeT::NameAndType:
            return lhs.nameId == rhs.nameId && lhs.typeId == rhs.typeId;
        case Constant::TypeT::MethodRef:
        case Constant::TypeT::FieldRef:
            return lhs.nameAndTypeId == rhs.nameAndTypeId && lhs.classId == rhs.classId;
    }
    return false;
}

Constant Constant::CreateUtf8(std::string const& content)
{
    Constant constant;
    constant.type = TypeT::Utf8;
    constant.utf8 = content;
    return constant;
}

Constant Constant::CreateInt(int value)
{
    Constant constant;
    constant.type = TypeT::Integer;
    constant.integer = value;
    return constant;
}

Constant Constant::CreateFloat(float value)
{
    Constant constant;
    constant.type = TypeT::Float;
    constant.floating = value;
    return constant;
}

Constant Constant::CreateString(IdT Utf8)
{
    Constant constant;
    constant.type = TypeT::String;
    constant.utf8Id = Utf8;
    return constant;
}

Constant Constant::CreateNaT(IdT nameId, IdT typeId)
{
    Constant constant;
    constant.type = TypeT::NameAndType;
    constant.nameId = nameId;
    constant.typeId = typeId;
    return constant;
}

Constant Constant::CreateClass(IdT classNameId)
{
    Constant constant;
    constant.type = TypeT::Class;
    constant.classNameId = classNameId;
    return constant;
}

Constant Constant::CreateFieldRef(IdT natId, IdT classId)
{
    Constant constant;
    constant.type = TypeT::FieldRef;
    constant.nameAndTypeId = natId;
    constant.classId = classId;
    return constant;
}

Constant Constant::CreateMethodRef(IdT natId, IdT classId)
{
    Constant constant;
    constant.type = TypeT::MethodRef;
    constant.nameAndTypeId = natId;
    constant.classId = classId;
    return constant;
}

std::vector<char> Constant::toBytes() {
    std::vector<char> bytes;
	std::vector<char> buffer;

	switch (type)
	{
	case Constant::TypeT::Utf8: {
		char const* c = utf8.c_str();
	    bytes.push_back((char)Constant::TypeT::Utf8);

		buffer = intToBytes(strlen(c));
	    bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		bytes.insert(bytes.end(), c, c + strlen(c));
		break;
	}
	case Constant::TypeT::Integer: {
		bytes.push_back((char)Constant::TypeT::Integer);

		buffer = intToBytes(integer);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		break;
	}
		
	case Constant::TypeT::Float: {
		bytes.push_back((char)Constant::TypeT::Float);

		buffer = floatToBytes(floating);
		bytes.insert(bytes.end(), buffer.begin(), buffer.end());
		break;
	}

	case Constant::TypeT::Class: {
		bytes.push_back((char)Constant::TypeT::Class);

		buffer = intToBytes(classNameId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}
	
	case Constant::TypeT::String: {
		bytes.push_back((char)Constant::TypeT::String);

		buffer = intToBytes(utf8Id);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}

	case Constant::TypeT::FieldRef: {
		bytes.push_back((char)Constant::TypeT::FieldRef);

		buffer = intToBytes(classId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

		buffer = intToBytes(nameAndTypeId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}

	case Constant::TypeT::MethodRef: {
		bytes.push_back((char)Constant::TypeT::MethodRef);
		
		buffer = intToBytes(classId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

		buffer = intToBytes(nameAndTypeId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}

	case Constant::TypeT::NameAndType: {
		bytes.push_back((char)Constant::TypeT::NameAndType);

		buffer = intToBytes(nameId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());

		buffer = intToBytes(typeId);
		bytes.insert(bytes.end(), buffer.begin() + 2, buffer.end());
		break;
	}
	}

	return bytes;
}

IdT ConstantPool::FindOrCreateUtf8(std::string_view utf8)
{
    const auto constant = Constant::CreateUtf8(std::string{ utf8 });
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateString(std::string_view str)
{
    const auto constant = Constant::CreateString(FindOrCreateUtf8(str));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateInt(int value)
{
    const auto constant = Constant::CreateInt(value);
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateFloat(float value)
{
    const auto constant = Constant::CreateFloat(value);
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateClass(std::string_view className)
{
    const auto constant = Constant::CreateClass(FindOrCreateUtf8(className));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateNaT(std::string_view name, std::string_view type)
{
    const auto constant = Constant::CreateNaT(FindOrCreateUtf8(name), FindOrCreateUtf8(type));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateFieldRef(std::string_view className, std::string_view name, std::string_view type)
{
    const auto constant = Constant::CreateFieldRef(FindOrCreateNaT(name, type), FindOrCreateClass(className));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindOrCreateMethodRef(std::string_view className, std::string_view name, std::string_view type)
{
    const auto constant = Constant::CreateMethodRef(FindOrCreateNaT(name, type), FindOrCreateClass(className));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}
