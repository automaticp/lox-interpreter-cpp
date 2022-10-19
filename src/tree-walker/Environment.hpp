#pragma once
#include <unordered_map>
#include <string>
#include "Value.hpp"
#include <utility>
#include <optional>


class Environment {
private:
    std::unordered_map<std::string, Value> env_;
    Environment* enclosing_{ nullptr };

public:
    Environment() = default;

    explicit Environment(Environment* enclosing) :
        enclosing_{ enclosing } {}

    void define(const std::string& name, Value value) {
        env_.insert_or_assign(name, std::move(value));
    }

    // optinal references when, eh?
    Value* get(const std::string& name) {
        if (env_.contains(name)) {
            return &env_[name];
        } else if (enclosing_) {
            return enclosing_->get(name);
        } else {
            return nullptr;
        }
    }

    Value* assign(const std::string& name, Value value) {
        if (env_.contains(name)) {
            env_[name] = std::move(value);
            return &env_[name];
        } else if (enclosing_) {
            return enclosing_->assign(name, std::move(value));
        } else {
            return nullptr;
        }
    }

};
