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


using String = std::string;
using Number = double;
using Boolean = bool;
using Nil = std::monostate;

class Object;
class Function;
class BuiltinFunction;
class ValueHandle;

using Value = std::variant<Nil, ValueHandle, Object, Function, BuiltinFunction, String, Number, Boolean>;
