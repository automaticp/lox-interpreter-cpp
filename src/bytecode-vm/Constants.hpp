#pragma once
#include "Value.hpp"
#include <vector>
#include <utility>

class Constants {
private:
    std::vector<Value> values_;

public:
    template<typename ...Args>
    size_t emplace_back(Args&&... args) {
        auto& elem = values_.emplace_back(std::forward<Args>(args)...);
        return &elem - values_.data();
    }

    Value& operator[](size_t idx) noexcept {
        assert(idx < values_.size() && "Out of bounds access");
        return values_[idx];
    }

    const Value& operator[](size_t idx) const noexcept {
        assert(idx < values_.size() && "Out of bounds access");
        return values_[idx];
    }

};
