#pragma once

#include "../semantic/entities.h"

#include <string>

using IdT = unsigned int;

class Constant
{
public:
    friend bool operator==(const Constant& lhs, const Constant& rhs);
    friend bool operator!=(const Constant& lhs, const Constant& rhs) { return !(lhs == rhs); }

    enum class TypeT : uint8_t
    {
        Utf8 = 1,
        Integer = 3,
        Float = 4,
        String = 8,
        NameAndType = 12,
        Class = 7,
        MethodRef = 10,
        FieldRef = 9
    } type{};

    std::string utf8{};
    int integer{};
    float floating{};

    IdT utf8Id{};

    IdT nameId{};
    IdT typeId{};

    IdT classNameId{};

    IdT nameAndTypeId{};
    IdT classId{};

    static Constant CreateUtf8(std::string const& content);

    static Constant CreateInt(int value);

    static Constant CreateFloat(float value);

    static Constant CreateString(IdT Utf8);

    static Constant CreateNaT(IdT nameId, IdT typeId);

    static Constant CreateClass(IdT classNameId);

    static Constant CreateFieldRef(IdT natId, IdT classId);

    static Constant CreateMethodRef(IdT natId, IdT classId);
};

struct JvmField
{
    IdT nameId;
    IdT typeId;
};

struct JvmMethod
{
    IdT nameId;
    IdT typeId;
    MethodEntity method;
};

struct ConstantPool
{
    std::vector<Constant> pool;

    IdT FindUtf8(std::string_view utf8);

    IdT FindString(std::string_view str);

    IdT FindInt(int i);

    IdT FindFloat(float i);

    IdT FindClass(std::string_view className);

    IdT FindNaT(std::string_view name, std::string_view type);

    IdT FindFieldRef(std::string_view className, std::string_view name, std::string_view type);

    IdT FindMethodRef(std::string_view className, std::string_view name, std::string_view type);
};

enum class AccessFlags : uint16_t
{
    Public = 0x0001,
    Protected = 0x0004,
    Private = 0x0002,
    Static = 0x0008,
    Final = 0x0010,
    Super = 0x0020
};


