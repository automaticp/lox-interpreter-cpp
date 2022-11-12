#pragma once
#include <span>
#include "ValueDecl.hpp"


using builtin_function_t = Value(*)(std::span<Value>);


Value builtin_clock(std::span<Value> args);
Value builtin_typename(std::span<Value> args);
Value builtin_rand(std::span<Value> args);
Value builtin_randint(std::span<Value> args);


class Environment;
class Resolver;

// Call on global Environment and when the Resolver is in the global scope
void setup_builtins(Environment& env, Resolver& resolver);
