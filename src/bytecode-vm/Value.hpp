#pragma once
#include "LiteralValue.hpp"
#include "Utils.hpp"

using Value = Number; // FIXME: change later


inline std::string to_string(const Value& value) {
    return std::string(num_to_string(value));
}
