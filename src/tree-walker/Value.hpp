#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include "Object.hpp"
#include "Utils.hpp"
#include <string_view>

using Value = std::variant<Object, std::string, double, bool, std::nullptr_t>;

template<typename ...Ts>
inline bool holds(const Value& value) {
    return (... || std::holds_alternative<Ts>(value));
}

inline bool holds_same(const Value& lhs, const Value& rhs) {
    return lhs.index() == rhs.index();
}


inline bool is_equal(const Value& lhs, const Value& rhs) {
    if (holds_same(lhs, rhs)) {
        return lhs == rhs;
    } else {
        return false;
    }
}


namespace detail {

struct ValueTypeNameVisitor {
    std::string_view operator()(const Object&) const {
        return "Object";
    }
    std::string_view operator()(const std::string&) const {
        return "String";
    }
    std::string_view operator()(const double&) const {
        return "Number";
    }
    std::string_view operator()(const bool&) const {
        return "Boolean";
    }
    std::string_view operator()(const std::nullptr_t&) const {
        return "Nil";
    }
};

} // namespace detail


inline std::string_view type_name(const Value& val) {
    return std::visit(detail::ValueTypeNameVisitor{}, val);
}


struct ValueToStringVisitor {
    std::string operator()(const Object& val) const { return "?Object?"; }
    std::string operator()(const std::string& val) const { return '"' + val + '"'; }
    std::string operator()(const double& val) const {
        return std::string(num_to_string(val));
    }
    std::string operator()(const bool& val) const { return { val ? "true" : "false" }; }
    std::string operator()(const std::nullptr_t& val) const { return { "nil" }; }
};

inline std::string to_string(const Value& value) {
    return std::visit(ValueToStringVisitor{}, value);
}
