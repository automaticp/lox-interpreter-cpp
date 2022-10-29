#pragma once
#include <variant>
#include <string>
#include <cstddef>


// Since 'Value' is not a class, but an alias,
// we cannot predeclare it as incomplete type:
//
// class Value; // not allowed
//
// Use this declaration instead.


using Nil = std::monostate;
using String = std::string;
using Number = double;
using Boolean = bool;

class Object;
class Function;
class BuiltinFunction;
class ValueHandle;

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
