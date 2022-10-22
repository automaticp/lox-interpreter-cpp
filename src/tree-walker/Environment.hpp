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

    void define(const std::string& name, Value value);

    Value* get(const std::string& name);

    Value* assign(const std::string& name, Value value);

};
