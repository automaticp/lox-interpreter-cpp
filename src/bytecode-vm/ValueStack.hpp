#pragma once
#include "Value.hpp"
#include <type_traits>
#include <vector>
#include <concepts>


class ValueStack{
private:
    std::vector<Value> stack_;

public:
    void push(const Value& val) noexcept(std::is_nothrow_copy_constructible_v<Value>) {
        stack_.emplace_back(val);
    }

    void push(Value&& val) noexcept(std::is_nothrow_move_constructible_v<Value>) {
        stack_.emplace_back(std::move(val));
    }

    Value pop() noexcept(std::is_nothrow_move_constructible_v<Value>) {
        Value temp{ std::move(stack_.back()) };
        stack_.pop_back();
        return temp;
    }

    Value& peek(size_t idx) noexcept {
        assert(stack_.rbegin() + idx < stack_.rend() && "Peek access out of bounds");
        return *(stack_.rbegin() + idx);
    }


};
