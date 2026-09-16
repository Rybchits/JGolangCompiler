#pragma once

#include <algorithm>
#include <array>
#include <string_view>

inline constexpr std::array<std::string_view, 8> BuiltInFunctions = {
    "print",
    "println",
    "len",
    "append",
    "readInt",
    "readFloat",
    "readString",
    "readBool",
};

inline bool IsBuiltInFunction(std::string_view identifier) {
    return std::find(BuiltInFunctions.begin(), BuiltInFunctions.end(), identifier) != BuiltInFunctions.end();
}
