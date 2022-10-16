#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include "Object.hpp"


using Value = std::variant<Object, std::string, double, bool, std::nullptr_t>;

template<typename ...Ts>
inline bool holds(const Value& value) {
    return (... || std::holds_alternative<Ts>(value));
}

inline bool holds_same(const Value& lhs, const Value& rhs) {
    return lhs.index() == rhs.index();
}


inline bool operator==(const Value& lhs, const Value& rhs) {
    if (holds_same(lhs, rhs)) {
        return lhs == rhs;
    } else {
        return false;
    }
}


