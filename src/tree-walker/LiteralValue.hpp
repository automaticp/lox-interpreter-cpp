#pragma once
#include <variant>
#include <string>

using LiteralValue = std::variant<std::string, double>;

struct LiteralToStringVisitor {
    std::string operator()(const std::string& val) { return val; }
    std::string operator()(const double& val) { return std::to_string(val); }
};
