#pragma once
#include <string>
#include <sstream>
#include <concepts>

class Expr;
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
    template<std::derived_from<Expr> ...Es>
    static std::string parenthesize(std::string_view name, const Es&... exprs) {
        std::stringstream ss;
        ss << '(' << name;
        (ss << ... << (std::string(" ") + exprs.accept(ExprASTPrinterVisitor{})));
        ss << ')';
        return ss.str();
    }
};


struct ExprInterpretVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
};

struct ExprResolveVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
};

struct ExprAnalyzeVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
};

