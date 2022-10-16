#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include "Object.hpp"


using Value = std::variant<Object, std::string, double, bool, std::nullptr_t>;




