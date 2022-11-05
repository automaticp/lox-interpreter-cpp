#pragma once
#include <string>
#include <utility>
#include <boost/unordered_map.hpp>
#include "ValueDecl.hpp"



class Environment {
private:
    boost::unordered_map<std::string, Value> map_;
    Environment* enclosing_{ nullptr };

public:
    Environment() = default;

    explicit Environment(Environment* enclosing) :
        enclosing_{ enclosing } {}

    Environment(Environment* enclosing, boost::unordered_map<std::string, Value> map);

    ValueHandle define(const std::string& name, Value value);

    ValueHandle get(const std::string& name);

    ValueHandle get_at(size_t distance, const std::string& name);

    ValueHandle assign(const std::string& name, Value value);

    ValueHandle assign_at(size_t distance, const std::string& name, Value value);

    Environment* enclosing() const noexcept { return enclosing_; }

    const auto& map() const noexcept { return map_; }

private:
    static ValueHandle make_handle(decltype(map_)::mapped_type& name);

    Environment* ancestor(size_t distance) {
        Environment* current{ this };
        while (current && distance) {
            current = current->enclosing_;
            --distance;
        }
        return current;
    }

};
