#pragma once
#include "Utils.hpp"
#include <variant>
#include <string>
#include <cstddef>
#include <boost/container/string.hpp>


using Nil = std::monostate;
using String = boost::container::string;
using Number = double;
using Boolean = bool;


using LiteralValue = std::variant<Nil, String, Number, Boolean>;

struct LiteralToStringVisitor {
    std::string operator()(const String& val) const { return std::string('"' + val + '"'); }
    std::string operator()(const Number& val) const { return std::string(num_to_string(val)); }
    std::string operator()(const Boolean& val) const { return { val ? "true" : "false" }; }
    std::string operator()(const Nil& /* val */) const { return { "nil" }; }
};

inline std::string to_string(const LiteralValue& literal) {
    return std::visit(LiteralToStringVisitor{}, literal);
}
