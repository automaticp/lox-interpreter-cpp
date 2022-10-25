#pragma once
#include "Value.hpp"


Value builtin_clock(std::span<Value> args);

Value builtin_typename(std::span<Value> args);


class Environment;
class Resolver;

// Call on global Environment and when the Resolver is in the global scope
void setup_builtins(Environment& env, Resolver& resolver);
