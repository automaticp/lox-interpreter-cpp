#pragma once
#include "Value.hpp"
#include "Errors.hpp"
#include <fmt/format.h>


class Expr;
class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class GroupedExpr;
class VariableExpr;
class AssignExpr;
class LogicalExpr;
class CallExpr;


class ErrorReporter;
class Interpreter;


struct ExprInterpreterVisitor {
    using return_type = Value;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
    return_type operator()(const VariableExpr& expr) const;
    return_type operator()(const AssignExpr& expr) const;
    return_type operator()(const LogicalExpr& expr) const;
    return_type operator()(const CallExpr& expr) const;

    ExprInterpreterVisitor(ErrorReporter& err, Environment& env, Interpreter& interpreter) :
        err{ err }, env{ env }, interpreter{ interpreter }  {}

    ErrorReporter& err;
    Environment& env;
    Interpreter& interpreter;
protected:

    return_type evaluate(const Expr& expr) const;
    return_type evaluate_without_decay(const Expr& expr) const;

    static bool is_truthful(const Value& value);

    template<typename T>
    void check_type(const Expr& expr, const Value& val) const {
        if (!val.is<T>()) {
            report_error_and_abort(
                InterpreterError::unexpected_type, expr,
                fmt::format("Expected {:s}, Encountered {:s}", type_name(Value{T{}}), type_name(val))
            );
        }
    }

    template<typename T1, typename T2>
    void check_type(const Expr& expr, const Value& val1, const Value& val2) const {
        check_type<T1>(expr, val1);
        check_type<T2>(expr, val2);
    }

    void abort_by_exception(InterpreterError type) const noexcept(false) {
        throw type;
    }

    void report_error(InterpreterError type, const Expr& expr, std::string_view details = "") const;

    void report_error_and_abort(InterpreterError type, const Expr& expr, std::string_view details = "") const {
        report_error(type, expr, details);
        abort_by_exception(type);
    }

    template<typename CallableValue>
    CallableValue& get_invokable(Value& callee, std::vector<Value>& args, const CallExpr& expr) const;
};

