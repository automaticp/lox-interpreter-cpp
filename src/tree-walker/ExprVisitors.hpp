#pragma once
#include <string>
#include <sstream>
#include <string_view>
#include <concepts>
#include <fmt/format.h>
#include "Environment.hpp"
#include "Value.hpp"
#include "Errors.hpp"

class IExpr;
class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class GroupedExpr;
class VariableExpr;
class AssignExpr;

struct ExprASTPrinterVisitor {
    using return_type = std::string;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
    return_type operator()(const VariableExpr& expr) const;
    return_type operator()(const AssignExpr& expr) const;

private:
    template<std::derived_from<IExpr> ...Es>
    static std::string parenthesize(std::string_view name, const Es&... exprs) {
        std::stringstream ss;
        ss << '(' << name;
        (ss << ... << (std::string(" ") + exprs.accept(ExprASTPrinterVisitor{})));
        ss << ')';
        return ss.str();
    }
};



class ErrorReporter;

struct ExprInterpreterVisitor {
    using return_type = Value;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
    return_type operator()(const VariableExpr& expr) const;
    return_type operator()(const AssignExpr& expr) const;

    explicit ExprInterpreterVisitor(ErrorReporter& err, Environment& env) : err_{ err }, env_{ env } {}

    ErrorReporter& err_;
protected:
    Environment& env_;

    return_type evaluate(const IExpr& expr) const;
    static bool is_truthful(const Value& value);

    template<typename T>
    void check_type(const IExpr& expr, const Value& val) const {
        if (!holds<T>(val)) {
            report_error_and_abort(
                InterpreterError::unexpected_type, expr,
                fmt::format("Expected {:s}, Encountered {:s}", type_name(Value{T{}}), type_name(val))
            );
        }
    }

    template<typename T1, typename T2>
    void check_type(const IExpr& expr, const Value& val1, const Value& val2) const {
        check_type<T1>(expr, val1);
        check_type<T2>(expr, val2);
    }

    void abort_by_exception(InterpreterError type) const noexcept(false) {
        throw type;
    }

    void report_error(InterpreterError type, const IExpr& expr, std::string_view details = "") const;

    void report_error_and_abort(InterpreterError type, const IExpr& expr, std::string_view details = "") const {
        report_error(type, expr, details);
        abort_by_exception(type);
    }
};


class Token;

struct ExprGetPrimaryTokenVisitor {
    using return_type = const Token&;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
    return_type operator()(const VariableExpr& expr) const;
    return_type operator()(const AssignExpr& expr) const;
};


struct ExprUserFriendlyNameVisitor {
    using return_type = std::string_view;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
    return_type operator()(const VariableExpr& expr) const;
    return_type operator()(const AssignExpr& expr) const;
};

