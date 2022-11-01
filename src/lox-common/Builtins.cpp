#include "Builtins.hpp"

#include "Environment.hpp"
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
    return std::string(type_name(args[0]));
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


