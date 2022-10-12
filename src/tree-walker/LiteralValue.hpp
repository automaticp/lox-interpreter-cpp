#pragma once
#include <variant>
#include <string>
#include <cstddef>

using LiteralValue = std::variant<std::string, double, bool, std::nullptr_t>;

struct LiteralToStringVisitor {
    std::string operator()(const std::string& val) { return val; }
    std::string operator()(const double& val) { return std::to_string(val); }
    std::string operator()(const bool& val) { return { val ? "true" : "false" }; }
    std::string operator()(const std::nullptr_t& val) { return { "nil" }; }
};

inline std::string to_string(const LiteralValue& literal) {
    return std::visit(LiteralToStringVisitor{}, literal);
}
