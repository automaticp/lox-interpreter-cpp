#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include "Utils.hpp"
#include "ValueDecl.hpp"


using LiteralValue = std::variant<Nil, String, Number, Boolean>;

struct LiteralToStringVisitor {
    std::string operator()(const std::string& val) const { return '"' + val + '"'; }
    std::string operator()(const double& val) const { return std::string(num_to_string(val)); }
    std::string operator()(const bool& val) const { return { val ? "true" : "false" }; }
    std::string operator()(const Nil& /* val */) const { return { "nil" }; }
};

inline std::string to_string(const LiteralValue& literal) {
    return std::visit(LiteralToStringVisitor{}, literal);
}
