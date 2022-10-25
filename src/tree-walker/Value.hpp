#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include <cassert>
#include <memory>
#include "Token.hpp"
#include "Utils.hpp"
#include <string_view>
#include "FieldName.hpp"
#include <functional>
#include <vector>
#include <cstdint>
#include <span>
#include <boost/unordered_map.hpp>
#include <fmt/format.h>
#include "Environment.hpp"
#include "ValueDecl.hpp"

// Define Callable and Object before anything triggers a template instantiation




// A wrapper type used in mutable contexts: assignment, calls of functions with closures.
// Should not be exposed to the user, must always decay before returning from evaluation.
class ValueHandle {
private:
    Value* handle_{ nullptr };

public:
    ValueHandle() = default;
    explicit ValueHandle(Value& target) noexcept;

    Value* pointer() const noexcept {
        return handle_;
    }
    Value& reference() const noexcept {
        assert(handle_);
        return *handle_;
    }
    // Copy construct from handle
    Value decay() const noexcept;

    // if (value_handle.wraps<Function>()) { ... }
    template<typename T>
    bool wraps() const noexcept {
        if (is_null()) { return false; }
        return std::holds_alternative<T>(reference());
    }

    bool is_null() const noexcept { return handle_; }
    operator bool() const noexcept { return handle_; }

    bool operator==(const ValueHandle& other) const noexcept {
        return handle_ == other.handle_ && handle_ != nullptr;
    }
};


class ExprInterpreterVisitor;

class FunStmt;

class Function {
private:
    Environment closure_;
    const FunStmt* declaration_;
    friend class ValueToStringVisitor;
public:
    Function(const FunStmt* declaration) : declaration_{ declaration } {}

    // Copy construct closure
    Function(const FunStmt* declaration, Environment closure) :
        declaration_{ declaration }, closure_{ std::move(closure) } {}


    Function(const Function&) = default;
    Function& operator=(const Function&) = default;
    Function(Function&&) = default;
    Function& operator=(Function&&) = default;


    Value operator()(const ExprInterpreterVisitor& interpreter, std::vector<Value>& args);

    size_t arity() const noexcept;

    bool operator==(const Function& other) const noexcept {
        return declaration_ == other.declaration_;
    }

};


class BuiltinFunction {
private:
    std::function<Value(std::span<Value>)> fun_;
    size_t arity_;
    std::string_view name_;
    friend class ValueToStringVisitor;
public:
    BuiltinFunction(std::string_view name, std::function<Value(std::span<Value>)> fun, size_t arity) :
        fun_{ std::move(fun) }, arity_{ arity }, name_{ name } {}

    Value operator()(std::span<Value> args);

    size_t arity() const noexcept { return arity_; }

    bool operator==(const BuiltinFunction& /* other */) const noexcept {
        return false;
    }
};




class Object {
private:
    boost::unordered_map<FieldName, Value> fields_;
public:
    bool operator==(const Object& /* other */) const {
        return false;
    }

    // Not defined yet
    Value operator()(const ExprInterpreterVisitor& interpreter, const std::vector<Value>& args);

};




template<typename ...Ts>
inline bool holds(const Value& value) {
    return (... || std::holds_alternative<Ts>(value));
}

inline bool holds_same(const Value& lhs, const Value& rhs) {
    return lhs.index() == rhs.index();
}

inline bool is_equal(const Value& lhs, const Value& rhs) {
    if (holds_same(lhs, rhs)) {
        return lhs == rhs;
    } else {
        return false;
    }
}


namespace detail {

struct ValueTypeNameVisitor {
    std::string_view operator()(const ValueHandle&) const {
        return "ValueHandle";
    }
    std::string_view operator()(const Object&) const {
        return "Object";
    }
    std::string_view operator()(const Function&) const {
        return "Function";
    }
    std::string_view operator()(const BuiltinFunction&) const {
        return "BuiltinFunction";
    }
    std::string_view operator()(const std::string&) const {
        return "String";
    }
    std::string_view operator()(const double&) const {
        return "Number";
    }
    std::string_view operator()(const bool&) const {
        return "Boolean";
    }
    std::string_view operator()(const std::nullptr_t&) const {
        return "Nil";
    }
};

} // namespace detail


inline std::string_view type_name(const Value& val) {
    return std::visit(detail::ValueTypeNameVisitor{}, val);
}


struct ValueToStringVisitor {
    std::string operator()(const ValueHandle& val) const {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Find me a better way, yeah
        return fmt::format("?ValueHandle {:x}?", reinterpret_cast<uintptr_t>(val.pointer()));
    }
    std::string operator()(const Object& /* val */) const {
        return "?Object?";
    }
    std::string operator()(const Function& val) const;
    std::string operator()(const BuiltinFunction& val) const;
    std::string operator()(const std::string& val) const {
        return fmt::format("\"{}\"", val);
    }
    std::string operator()(const double& val) const {
        return std::string(num_to_string(val));
    }
    std::string operator()(const bool& val) const {
        return { val ? "true" : "false" };
    }
    std::string operator()(const std::nullptr_t& /* val */) const {
        return { "nil" };
    }
};

inline std::string to_string(const Value& value) {
    return std::visit(ValueToStringVisitor{}, value);
}
