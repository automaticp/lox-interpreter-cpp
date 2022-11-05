#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include <boost/container/string.hpp>

using Nil = std::monostate;
class Object;
class ValueHandle;
class Function;
class BuiltinFunction;
// boost::container::string guarantees a 24-byte footprint
// and 23-byte short-string storage on 64-bit machines,
// whereas libstdc++ std::string is 32-byte with only 16 bytes
// for short-string optimization.
using String = boost::container::string;
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
