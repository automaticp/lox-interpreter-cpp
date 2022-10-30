#pragma once
#include <variant>
#include <string>
#include <cstddef>

using Nil = std::monostate;
class Object;
class ValueHandle;
class Function;
class BuiltinFunction;
using String = std::string;
using Number = double;
using Boolean = bool;


using ValueVariant = std::variant<Nil, ValueHandle, Object, Function, BuiltinFunction, String, Number, Boolean>;

class Value;

// A user-friendly alternative to variant::index().
// Probably don't do 'using enum ValueType;'.
enum class ValueType : size_t {
    Nil = 0,
    ValueHandle,
    Object,
    Function,
    BuiltinFunction,
    String,
    Number,
    Boolean
};
