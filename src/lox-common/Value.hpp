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
#include "VariantWrapper.hpp"


// This file defines the underlying Value variants
// and the Value wrapper class itself.
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
    std::string_view operator()(const String&) const {
        return "String";
    }
    std::string_view operator()(const Number&) const {
        return "Number";
    }
    std::string_view operator()(const Boolean&) const {
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
    std::string operator()(const String& val) const {
        return fmt::format("\"{}\"", val);
    }
    std::string operator()(const Number& val) const {
        return std::string(num_to_string(val));
    }
    std::string operator()(const Boolean& val) const {
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
    explicit ValueHandle(Value& target) noexcept;

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
    bool wraps() const noexcept;

    template<typename T>
    T& unwrap_to() const noexcept;

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




class FunStmt;


class Function {
private:
    class Impl {
    private:
        Environment closure_;
        const FunStmt* declaration_;
        friend Function;

    public:
        Impl(const FunStmt* declaration) : declaration_{ declaration } {}

        // Copy construct closure
        Impl(const FunStmt* declaration, Environment closure) :
            declaration_{ declaration }, closure_{ std::move(closure) } {}

    }; // class Impl


    std::shared_ptr<Impl> pimpl_;
    // Never emplace the copy of the Function into it's closure.
    // That leaks memory because the closure keeps the Impl alive.

public:
    template<typename ...Args>
    Function(Args&&... args) :
        pimpl_{ std::make_shared<Impl>(std::forward<Args>(args)...) }
    {}

    // You'd think that a Function type should define it's call operator,
    // But the details of the implementation will be heavily
    // dependent on the interpreter/backend. Originally, this class
    // had such call method, that looked like this:
    //
    // Value operator()(const ExprInterpreterVisitor& interpreter, std::vector<Value>& args);
    //
    // which introduced backend dependency due to ExprInterpreterVisitor being a parameter.
    // Instead, we declare a template 'operator()', that can be specialized for
    // a particular backend (see below). Use that instead.

    // Specialize like this:
    //
    // template<>
    // Value Function::operator()<Interpreter>(const Interpreter& intrp, std::span<Value> args) {
    //     ...
    // }
    //
    template<typename BackendT>
    Value operator()(BackendT&, std::span<Value> args);


    size_t arity() const noexcept;

    Environment& closure() noexcept { return pimpl_->closure_; }

    const FunStmt* declaration() const noexcept { return pimpl_->declaration_; }

    bool operator==(const Function& other) const noexcept {
        return pimpl_.get() == other.pimpl_.get();
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


    // FIXME: to span or not to span?
    template<typename BackendT>
    Value operator()(BackendT&, std::span<Value> args);

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
    template<typename BackendT>
    Value operator()(BackendT&, std::span<Value> args);

};



// A wrapper class around a ValueVariant
// that defines a more suitable interface
// for common use patterns.
class Value : public VariantWrapper<Value, ValueVariant> {
public:
    using VariantWrapper<Value, ValueVariant>::VariantWrapper;

    // Nil constructor (aka std::monostate)
    Value() = default;
    // Value() : variant_{ Nil{} } {}

    ValueType type() const noexcept {
        assert(!variant_.valueless_by_exception());
        return static_cast<ValueType>(index());
    }

    bool operator==(const Value& other) const noexcept {
        return variant_ == other.variant_;
    }
};



// Value 'Methods'


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




// Other definitions, that rely on Value

template<typename T>
bool ValueHandle::wraps() const noexcept {
    if (is_null()) { return false; }
    return reference().is<T>();
}


template<typename T>
T& ValueHandle::unwrap_to() const noexcept {
    assert(wraps<T>());
    return reference().as<T>();
}
