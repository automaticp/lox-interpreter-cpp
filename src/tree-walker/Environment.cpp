#include "Environment.hpp"
#include "Value.hpp"
#include <utility>

template class boost::unordered_map<std::string, Value>;


Environment::Environment(Environment* enclosing, boost::unordered_map<std::string, Value> map) :
        enclosing_{ enclosing }, map_{ std::move(map) } {}



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


Value* Environment::get_at(size_t distance, const std::string& name) {
    assert(ancestor(distance));
    assert(ancestor(distance)->map_.contains(name));
    return &ancestor(distance)->map_[name];
}


Value* Environment::assign_at(size_t distance, const std::string& name, Value value) {
    assert(ancestor(distance));
    assert(ancestor(distance)->map_.contains(name));
    auto& value_ref = ancestor(distance)->map_[name];
    value_ref = std::move(value);
    return &value_ref;
}
