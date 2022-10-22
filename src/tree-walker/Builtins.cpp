#include "Builtins.hpp"

#include <chrono>
#include "Environment.hpp"
#include "Value.hpp"


Value builtin_clock(std::span<Value> args) {
    auto time_point = std::chrono::high_resolution_clock::now();
    using seconds = std::chrono::duration<double>;
    return seconds(time_point.time_since_epoch()).count();
}





void setup_builtins(Environment& env) {
    env.define("clock", BuiltinFunction{ "clock", builtin_clock, 0 });
}
