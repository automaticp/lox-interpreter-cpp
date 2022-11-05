#include "Environment.hpp"
#include "Value.hpp"
#include <utility>

template class boost::unordered_map<std::string, Value>;


Environment::Environment(Environment* enclosing, boost::unordered_map<std::string, Value> map) :
        enclosing_{ enclosing }, map_{ std::move(map) } {}


// Returns a handle to a Value in a local map.
// Does not recurse to the enclosing environments.
//
// If the value at map_[name] is a ValueHandle, decays it
// so as to not form the handle to a handle.
// Otherwise, just returns a handle to the Value.
ValueHandle Environment::make_handle(decltype(map_)::mapped_type& target) {
    return ValueHandle{ decay(target) };
}


// Retruns a handle to the new element
ValueHandle Environment::define(const std::string& name, Value value) {
    auto [it, was_inserted] = map_.insert_or_assign(name, std::move(value));
    return make_handle(it->second);
}


ValueHandle Environment::get(const std::string& name) {
    if (map_.contains(name)) {
        return make_handle(map_[name]);
    } else if (enclosing_) {
        return enclosing_->get(name);
    } else {
        return {};
    }
}

ValueHandle Environment::assign(const std::string& name, Value value) {
    if (map_.contains(name)) {
        map_[name] = std::move(value);
        return make_handle(map_[name]);
    } else if (enclosing_) {
        return enclosing_->assign(name, std::move(value));
    } else {
        return {};
    }
}


ValueHandle Environment::get_at(size_t distance, const std::string& name) {
    assert(ancestor(distance));
    assert(ancestor(distance)->map_.contains(name));
    return make_handle(ancestor(distance)->map_[name]);
}


ValueHandle Environment::assign_at(size_t distance, const std::string& name, Value value) {
    assert(ancestor(distance));
    assert(ancestor(distance)->map_.contains(name));
    Value& value_ref = ancestor(distance)->map_[name];
    value_ref = std::move(value);
    return make_handle(value_ref);
}
