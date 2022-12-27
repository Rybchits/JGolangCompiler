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

Constant Constant::CreateInt(IntT value)
{
    Constant constant;
    constant.type = TypeT::Integer;
    constant.integer = value;
    return constant;
}

Constant Constant::CreateFloat(FloatT value)
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

IdT ConstantPool::FindUtf8(std::string_view utf8)
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

IdT ConstantPool::FindString(std::string_view str)
{
    const auto constant = Constant::CreateString(FindUtf8(str));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindInt(IntT value)
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

IdT ConstantPool::FindFloat(FloatT value)
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

IdT ConstantPool::FindClass(std::string_view className)
{
    const auto constant = Constant::CreateClass(FindUtf8(className));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindNaT(std::string_view name, std::string_view type)
{
    const auto constant = Constant::CreateNaT(FindUtf8(name), FindUtf8(type));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindFieldRef(std::string_view className, std::string_view name, std::string_view type)
{
    const auto constant = Constant::CreateFieldRef(FindNaT(name, type), FindClass(className));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}

IdT ConstantPool::FindMethodRef(std::string_view className, std::string_view name, std::string_view type)
{
    const auto constant = Constant::CreateMethodRef(FindNaT(name, type), FindClass(className));
    const auto foundIter = std::find(pool.begin(), pool.end(), constant);
    if (foundIter == pool.end())
    {
        pool.push_back(constant);
        return pool.end() - pool.begin();
    }
    return foundIter - pool.begin() + 1;
}
