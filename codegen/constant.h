#pragma once

#include "../utils/bytes.h"

#include <string>

using IdT = unsigned int;

class Constant
{
public:
    friend bool operator==(const Constant& lhs, const Constant& rhs);
    friend bool operator!=(const Constant& lhs, const Constant& rhs) { return !(lhs == rhs); }

    static Constant CreateUtf8(std::string const& content);

    static Constant CreateInt(int value);

    static Constant CreateFloat(float value);

    static Constant CreateString(IdT Utf8);

    static Constant CreateNaT(IdT nameId, IdT typeId);

    static Constant CreateClass(IdT classNameId);

    static Constant CreateFieldRef(IdT natId, IdT classId);

    static Constant CreateMethodRef(IdT natId, IdT classId);

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

    std::vector<char> toBytes();

private:
    std::string utf8{};
    int integer{};
    float floating{};

    IdT utf8Id{};

    IdT nameId{};
    IdT typeId{};

    IdT classNameId{};

    IdT nameAndTypeId{};
    IdT classId{};
};


struct ConstantPool
{
    std::vector<Constant> pool;

    IdT FindOrCreateUtf8(std::string_view utf8);

    IdT FindOrCreateString(std::string_view str);

    IdT FindOrCreateInt(int i);

    IdT FindOrCreateFloat(float i);

    IdT FindOrCreateClass(std::string_view className);

    IdT FindOrCreateNaT(std::string_view name, std::string_view type);

    IdT FindOrCreateFieldRef(std::string_view className, std::string_view name, std::string_view type);

    IdT FindOrCreateMethodRef(std::string_view className, std::string_view name, std::string_view type);
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


enum class ArrayType : uint8_t
{
    Boolean = 4,
    Char = 5,
    Float = 6,
    Double = 7,
    Byte = 8,
    Short = 9,
    Int = 10,
    Long = 11
};

