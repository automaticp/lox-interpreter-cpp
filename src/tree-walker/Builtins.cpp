#include "Builtins.hpp"

#include "Environment.hpp"
#include "Resolver.hpp"
#include "Value.hpp"
#include <chrono>
#include <utility>
#include <functional>
#include <random>

Value builtin_clock(std::span<Value> /* args */) {
    auto time_point = std::chrono::high_resolution_clock::now();
    using seconds = std::chrono::duration<double>;
    return seconds(time_point.time_since_epoch()).count();
}

Value builtin_typename(std::span<Value> args) {
    return String(type_name(args[0]));
}

Value builtin_rand(std::span<Value> /* args */) {
    thread_local std::mt19937 engine{};
    thread_local std::uniform_real_distribution<double> dist{};
    return dist(engine);
}

Value builtin_randint(std::span<Value> args) {
    thread_local std::mt19937 engine{};
    if (!args[0].is<Number>() || !args[1].is<Number>()) {
        return {};
    }
    const auto min = static_cast<long long>(args[0].as<Number>());
    const auto max = static_cast<long long>(args[1].as<Number>());
    std::uniform_int_distribution<long long int> dist{ min, max };
    return static_cast<Number>(dist(engine));
}





// Call on global Environment and when the Resolver is in the global scope
void setup_builtins(Environment& env, Resolver& resolver) {

    auto define_builtin =
        [&env, &resolver](
            const char* name, builtin_function_t fun, size_t arity
        ) {

        // name is passed by const char* due to BuiltinFunction storing a const char*.
        std::string name_string{ name };

        bool success = resolver.declare(name_string);
        assert(success && "This should definetly not happen.");
        resolver.define(name_string);

        env.define(name_string, BuiltinFunction{ name, fun, arity });

        // Also, just wondering, why the std::string(std::string_view) constructor is explicit?
        // Like, annoying.
    };


    define_builtin("clock", builtin_clock, 0);
    define_builtin("typename", builtin_typename, 1);
    define_builtin("rand", builtin_rand, 0);
    define_builtin("randint", builtin_randint, 2);

}



