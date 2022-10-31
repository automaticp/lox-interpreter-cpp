#pragma once
#include <span>
#include "ValueDecl.hpp"


Value builtin_clock(std::span<Value> args);
Value builtin_typename(std::span<Value> args);
Value builtin_rand(std::span<Value> args);
Value builtin_randint(std::span<Value> args);

