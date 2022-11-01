#pragma once
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"


class Frontend {
// Put code here
};




// Call on global Environment and when the Resolver is in the global scope
void setup_builtins(Environment& env, Resolver& resolver);
