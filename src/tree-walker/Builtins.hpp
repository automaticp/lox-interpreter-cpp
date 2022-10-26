#pragma once
#include <span>
#include "ValueDecl.hpp"



class Environment;
class Resolver;

// Call on global Environment and when the Resolver is in the global scope
void setup_builtins(Environment& env, Resolver& resolver);
