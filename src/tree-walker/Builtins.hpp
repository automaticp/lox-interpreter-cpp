#pragma once
#include "Value.hpp"


Value builtin_clock(std::span<Value> args);



class Environment;

void setup_builtins(Environment& env);