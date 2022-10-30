#include "Environment.hpp"
#include "Value.hpp"
#include <utility>

template class boost::unordered_map<std::string, Value>;


Environment::Environment(Environment* enclosing, boost::unordered_map<std::string, Value> map) :
        enclosing_{ enclosing }, map_{ std::move(map) } {}



// Retruns a handle to the new element
ValueHandle Environment::define(const std::string& name, Value value) {
    auto [it, was_inserted] = map_.insert_or_assign(name, std::move(value));
    return ValueHandle{ it->second };
}


// Due to ValueHandle having different, but deceivingly
// similar constructors, which ones of them are invoked
// is clarified in the comments below.

ValueHandle Environment::get(const std::string& name) {
    if (map_.contains(name)) {
        // ValueHandle(Value&) **From ref**
        return ValueHandle{ map_[name] };
    } else if (enclosing_) {
        // ValueHandle(const ValueHandle&) **Copy**
        return enclosing_->get(name);
    } else {
        // ValueHandle() **Empty handle**
        return {};
    }
}

ValueHandle Environment::assign(const std::string& name, Value value) {
    if (map_.contains(name)) {
        map_[name] = std::move(value);
        return ValueHandle{ map_[name] }; // From ref
    } else if (enclosing_) {
        return enclosing_->assign(name, std::move(value)); // Copy
    } else {
        return {}; // Empty handle
    }
}


ValueHandle Environment::get_at(size_t distance, const std::string& name) {
    assert(ancestor(distance));
    assert(ancestor(distance)->map_.contains(name));
    return ValueHandle{ ancestor(distance)->map_[name] }; // From ref
}


ValueHandle Environment::assign_at(size_t distance, const std::string& name, Value value) {
    assert(ancestor(distance));
    assert(ancestor(distance)->map_.contains(name));
    Value& value_ref = ancestor(distance)->map_[name];
    value_ref = std::move(value);
    return ValueHandle{ value_ref }; // From ref
}
