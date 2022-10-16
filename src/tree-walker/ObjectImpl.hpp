#pragma once
#include <unordered_map>
#include "FieldName.hpp"
#include "Value.hpp"


class ObjectImpl {
private:
    std::unordered_map<FieldName, Value> fields_;
public:
    ObjectImpl() = default;
};


