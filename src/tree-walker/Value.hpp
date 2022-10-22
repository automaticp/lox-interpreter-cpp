#pragma once
#include <variant>
#include <string>
#include <cstddef>
#include <memory>
#include "Token.hpp"
#include "Utils.hpp"
#include <string_view>
#include <unordered_map>
#include "FieldName.hpp"
#include <functional>
#include <vector>
#include <span>
#include "Environment.hpp"
#include "ValueDecl.hpp"

// Define Callable and Object before anything triggers a template instantiation


class ExprInterpreterVisitor;

class FunStmt;

class Function {
private:
    Environment closure_;
    const FunStmt* declaration_;
    friend class ValueToStringVisitor;
public:
    Function(const FunStmt* declaration) : declaration_{ declaration } {}

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

public:
    BuiltinFunction(std::function<Value(std::span<Value>)> fun, size_t arity) :
        fun_{ std::move(fun) }, arity_{ arity } {}

    Value operator()(std::span<Value> args);

    size_t arity() const noexcept { return arity_; }

    bool operator==(const BuiltinFunction& other) const noexcept {
        return false;
    }
};



class ObjectImpl;

class Object {
private:
    std::unique_ptr<ObjectImpl> pimpl_;
public:
    Object();
    Object(const Object&);
    Object& operator=(const Object&);
    // Defined is cpp file, as otherwise unique_ptr
    // tries to generate ObjectImpl's destructor here.
    // Which doesn't work due to class being incomplete.
    Object& operator=(Object &&);
    Object(Object &&);
    ~Object();

    bool operator==(const Object& other) const {
        return false;
    }

    Value operator()(const ExprInterpreterVisitor& interpreter, const std::vector<Value>& args);

private:
    ObjectImpl& impl() const noexcept {
        return *pimpl_;
    }
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
    std::string operator()(const Object& val) const { return "?Object?"; }
    std::string operator()(const Function& val) const;
    std::string operator()(const BuiltinFunction& val) const { return "?BuiltinFunction?"; }
    std::string operator()(const std::string& val) const { return '"' + val + '"'; }
    std::string operator()(const double& val) const {
        return std::string(num_to_string(val));
    }
    std::string operator()(const bool& val) const { return { val ? "true" : "false" }; }
    std::string operator()(const std::nullptr_t& val) const { return { "nil" }; }
};

inline std::string to_string(const Value& value) {
    return std::visit(ValueToStringVisitor{}, value);
}
