#pragma once
#include <string>
#include <sstream>
#include <concepts>
#include "Value.hpp"

class IExpr;
class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class GroupedExpr;

struct ExprASTPrinterVisitor {
    using return_type = std::string;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;

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


struct ExprInterpreterVisitor {
    using return_type = Value;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;

private:
    return_type evaluate(const IExpr& expr) const;
    static bool is_truthful(const Value& value);

    template<typename T>
    const T& try_cast(const Value& value) const {
        if (!holds<T>(value)) {
            abort_by_exception(/* InterpreterError::unexpected_type */);
        }
        return std::get<T>(value);
    }

    void abort_by_exception(/* InterpreterError type */) const noexcept(false) {
        // throw type;
        throw;
    }
};

struct ExprResolveVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const {}
    return_type operator()(const UnaryExpr& expr) const {}
    return_type operator()(const BinaryExpr& expr) const {}
    return_type operator()(const GroupedExpr& expr) const {}
};

struct ExprAnalyzeVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const {}
    return_type operator()(const UnaryExpr& expr) const {}
    return_type operator()(const BinaryExpr& expr) const {}
    return_type operator()(const GroupedExpr& expr) const {}
};

