#pragma once
#include <span>
#include "ValueDecl.hpp"


using builtin_function_t = Value(*)(std::span<Value>);


Value builtin_clock(std::span<Value> args);
Value builtin_typename(std::span<Value> args);
Value builtin_rand(std::span<Value> args);
Value builtin_randint(std::span<Value> args);

