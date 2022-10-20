#pragma once
#include <unordered_map>
#include <string>
#include "Value.hpp"
#include <utility>
#include <optional>


class Environment {
private:
    std::unordered_map<std::string, Value> map_;
    Environment* enclosing_{ nullptr };

public:
    Environment() = default;

    explicit Environment(Environment* enclosing) :
        enclosing_{ enclosing } {}

    void define(const std::string& name, Value value) {
        map_.insert_or_assign(name, std::move(value));
    }

    // optinal references when, eh?
    Value* get(const std::string& name) {
        if (map_.contains(name)) {
            return &map_[name];
        } else if (enclosing_) {
            return enclosing_->get(name);
        } else {
            return nullptr;
        }
    }

    Value* assign(const std::string& name, Value value) {
        if (map_.contains(name)) {
            map_[name] = std::move(value);
            return &map_[name];
        } else if (enclosing_) {
            return enclosing_->assign(name, std::move(value));
        } else {
            return nullptr;
        }
    }

};
