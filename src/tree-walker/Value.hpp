#pragma once
#include <concepts>
#include <variant>
#include <string>
#include <cstddef>
#include <utility>
#include <cassert>
#include <memory>
#include "Token.hpp"
#include "Utils.hpp"
#include <string_view>
#include "FieldName.hpp"
#include <type_traits>
#include <functional>
#include <vector>
#include <cstdint>
#include <span>
#include <boost/unordered_map.hpp>
#include <fmt/format.h>
#include "Environment.hpp"
#include "ValueDecl.hpp"


// This file defines the underlying Value variants.
// The definitions here could be incomplete and/or inconsistent,
// as the exact semantics of the language and it's implementation
// are under constant re-evaluation.


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
    std::string_view operator()(const Nil&) const {
        return "Nil";
    }
};

struct ValueToStringVisitor {
    std::string operator()(const ValueHandle& val) const;
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
    std::string operator()(const Nil& /* val */) const {
        return { "nil" };
    }
};


} // namespace detail



// Define Function, Object, etc. before anything triggers a template instantiation


// A wrapper type used in mutable contexts: assignment, calls of functions with closures.
// Should not be exposed to the user, must always decay before returning from evaluation.
class ValueHandle {
private:
    Value* handle_{ nullptr };

public:
    ValueHandle() = default;
    ValueHandle(Value& target) noexcept;

    Value* pointer() const noexcept {
        return handle_;
    }
    Value& reference() const noexcept {
        assert(handle_);
        return *handle_;
    }
    // Copy construct from handle.
    // See also free function named decay().
    Value decay() const noexcept;

    // if (value_handle.wraps<Function>()) { ... }
    template<typename T>
    bool wraps() const noexcept {
        if (is_null()) { return false; }
        return std::holds_alternative<T>(reference());
    }

    template<typename T>
    T& unwrap_to() const noexcept {
        assert(wraps<T>());
        return std::get<T>(reference());
    }

    bool is_null() const noexcept { return !handle_; }
    operator bool() const noexcept { return handle_; }

    operator Value*() const noexcept { return handle_; }

    Value& operator*() const noexcept {
        return reference();
    }

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
    friend class detail::ValueToStringVisitor;
public:
    Function(const FunStmt* declaration) : declaration_{ declaration } {}

    // Copy construct closure
    Function(const FunStmt* declaration, Environment closure) :
        declaration_{ declaration }, closure_{ std::move(closure) } {}


    Value operator()(const ExprInterpreterVisitor& interpreter, std::vector<Value>& args);

    size_t arity() const noexcept;

    Environment& closure() noexcept { return closure_; }

    bool operator==(const Function& other) const noexcept {
        return declaration_ == other.declaration_;
    }

};





class BuiltinFunction {
private:
    std::function<Value(std::span<Value>)> fun_;
    size_t arity_;
    std::string_view name_;
    friend class detail::ValueToStringVisitor;
public:
    BuiltinFunction(std::string_view name, std::function<Value(std::span<Value>)> fun, size_t arity) :
        fun_{ std::move(fun) }, arity_{ arity }, name_{ name } {}

    Value operator()(std::span<Value> args);

    size_t arity() const noexcept { return arity_; }

    bool operator==(const BuiltinFunction& /* other */) const noexcept {
        return false;
    }
};



// Not implemented yet.
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








template<typename T>
concept ValueVisitor = requires(const T& visitor, const ValueVariant& value) {
    std::visit(T{}, value);
    std::visit(visitor, value);
};



class Value {
private:
    ValueVariant variant_;

public:
    template<typename ...Args>
    explicit(false) Value(Args&&... args) : variant_{ args... } {}

    auto index() const noexcept {
        return variant_.index();
    }

    ValueType type() const noexcept {
        assert(!variant_.valueless_by_exception());
        return static_cast<ValueType>(index());
    }


    template<typename T>
    bool is() const noexcept {
        return std::holds_alternative<T>(variant_);
    }

    template<typename ...Ts>
    bool is_any_of() const noexcept {
        return (... || is<Ts>());
    }

    template<typename T>
    T& as() { return std::get<T>(variant_); }

    template<typename T>
    T as() && { return std::move(std::get<T>(variant_)); }

    template<typename T>
    const T& as() const { return std::get<T>(variant_); }


    template<ValueVisitor T>
    auto accept(const T& visitor) const {
        return std::visit(visitor, variant_);
    }

};



// Value 'Methods'


template<typename ...Ts>
inline bool holds(const Value& value) {
    return (... || value.is<Ts>());
}

inline bool holds_same(const Value& lhs, const Value& rhs) {
    return lhs.index() == rhs.index();
}


inline std::string_view type_name(const Value& value) {
    return value.accept(detail::ValueTypeNameVisitor{});
}

inline std::string to_string(const Value& value) {
    return value.accept(detail::ValueToStringVisitor{});
}



// Decays the ValueHandle into the underlying Value,
// and transparently forwards other Value variants.
//
template<typename ValueT> requires std::same_as<std::remove_reference_t<ValueT>, Value>
ValueT decay(ValueT&& val) {
    if (val.template is<ValueHandle>()) {
        // Do not forward this, as it will move the underlying value,
        // 'breaking' the Value in the storage of it's Environment.
        // Instead, copy construct when val is captured from rvalue.
        return val.template as<ValueHandle>().reference();
    } else {
        return std::forward<ValueT>(val);
    }
}
// Due to reference collapsing, ValueT is deduced
// to Value& when the argument is an lvalue,
// and Value (not Value&&) when the argument is an rvalue.
// Equivalent to defining:
//
// Value& decay(Value& val) { ... }
// Value decay(Value&& val) { ... }
//
// Do not even think of returning ValueT&& -
// that is a dangling reference to a local variable,
// as the lifetime of an rvalue is extended only once:
//
// Value&& decay(Value&& val) { // <-- lifetime extended to this scope
//     ...
//     return val;
// } // <-- val is destroyed here.
//


