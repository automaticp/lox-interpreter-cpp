#include "Environment.hpp"
#include "Value.hpp"



void Environment::define(const std::string& name, Value value) {
    map_.insert_or_assign(name, std::move(value));
}

// optinal references when, eh?
Value* Environment::get(const std::string& name) {
    if (map_.contains(name)) {
        return &map_[name];
    } else if (enclosing_) {
        return enclosing_->get(name);
    } else {
        return nullptr;
    }
}

Value* Environment::assign(const std::string& name, Value value) {
    if (map_.contains(name)) {
        map_[name] = std::move(value);
        return &map_[name];
    } else if (enclosing_) {
        return enclosing_->assign(name, std::move(value));
    } else {
        return nullptr;
    }
}
