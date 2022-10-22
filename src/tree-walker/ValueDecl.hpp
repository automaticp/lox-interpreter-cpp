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


class Object;
class Function;
class Environment;
class BuiltinFunction;

using Value = std::variant<Object, Function, BuiltinFunction, std::string, double, bool, std::nullptr_t>;
