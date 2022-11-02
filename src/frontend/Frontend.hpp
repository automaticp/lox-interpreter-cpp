#pragma once
#include "Scanner.hpp"
#include "Parser.hpp"



class Frontend {
// Put code here
};


class Environment;
class Resolver;

// Call on global Environment and when the Resolver is in the global scope
void setup_builtins(Environment& env, Resolver& resolver);
