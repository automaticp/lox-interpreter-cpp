#include "Builtins.hpp"

#include "Environment.hpp"
#include "Value.hpp"
#include "Resolver.hpp"
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
    return std::string(type_name(args[0]));
}

Value builtin_rand(std::span<Value> /* args */) {
    thread_local std::mt19937 engine{};
    thread_local std::uniform_real_distribution<double> dist{};
    return dist(engine);
}

Value builtin_randint(std::span<Value> args) {
    thread_local std::mt19937 engine{};
    if (!holds<double>(args[0]) || !holds<double>(args[1])) {
        return nullptr;
    }
    const auto min = static_cast<long long>(std::get<double>(args[0]));
    const auto max = static_cast<long long>(std::get<double>(args[1]));
    std::uniform_int_distribution<long long int> dist{ min, max };
    return static_cast<double>(dist(engine));
}



void setup_builtins(Environment& env, Resolver& resolver) {

    auto define_builtin =
        [&env, &resolver](
            std::string_view name, std::function<Value(std::span<Value>)> fun, size_t arity
        ) {

        // name is passed by string_view due to BuiltinFunction storing a string_view.
        // If I were to pass const string&, then the view would be dangling.
        std::string name_string{ name };

        bool success = resolver.declare(name_string);
        assert(success && "This should definetly not happen.");
        resolver.define(name_string);

        env.define(std::string(name), BuiltinFunction{ name, std::move(fun), arity });

        // Also, just wondering, why the std::string(std::string_view) constructor is explicit?
        // Like, annoying.
    };


    define_builtin("clock", builtin_clock, 0);
    define_builtin("typename", builtin_typename, 1);
    define_builtin("rand", builtin_rand, 0);
    define_builtin("randint", builtin_randint, 2);

}


