#include "Frontend.hpp"

#include "Value.hpp"
#include "Builtins.hpp"
#include "Environment.hpp"
#include "Resolver.hpp"
#include <string_view>
#include <functional>
#include <span>













// Call on global Environment and when the Resolver is in the global scope
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


