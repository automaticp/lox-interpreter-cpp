#pragma once
#include <unordered_map>
#include <string>
#include "Value.hpp"
#include <utility>
#include <optional>


class Environment {
private:
    std::unordered_map<std::string, Value> env_;

public:
    Environment() = default;

    void define(const std::string& name, Value value) {
        env_.insert_or_assign(name, std::move(value));
    }

    // optinal references when, eh?
    Value* get(const std::string& name) {
        if (env_.contains(name)) {
            return &env_[name];
        } else {
            return nullptr;
        }
    }

    Value* assign(const std::string& name, Value value) {
        if (env_.contains(name)) {
            env_[name] = value;
            return &env_[name];
        } else {
            return nullptr;
        }
    }

};
