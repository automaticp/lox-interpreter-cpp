#pragma once
#include <variant>
#include <utility>
#include <memory>
#include <string>
#include <sstream>
#include <concepts>
#include "LiteralValue.hpp"
#include "Token.hpp"
#include "Visitable.hpp"


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



// Extend these two lists to add more visitors
template<typename CRTP>
using FullyVisitableExpr = Visitable<
    CRTP, ExprInterpretVisitor, ExprResolveVisitor, ExprAnalyzeVisitor, ExprASTPrinterVisitor
>;

template<typename CRTP>
using FullyVisitableExprInterface = IVisitable<
    CRTP, ExprInterpretVisitor, ExprResolveVisitor, ExprAnalyzeVisitor, ExprASTPrinterVisitor
>;





struct Expr : FullyVisitableExprInterface<Expr> {
public:
    virtual ~Expr() = default;
};


struct LiteralExpr : Expr, FullyVisitableExpr<LiteralExpr> {
public:
    LiteralValue value;

    LiteralExpr(LiteralValue value) :
        value{ std::move(value) } {}
};


struct UnaryExpr : Expr, FullyVisitableExpr<UnaryExpr> {
public:
    TokenType op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(TokenType op, std::unique_ptr<Expr> expr) :
        op{ op }, operand{ std::move(expr) } {}
};


struct BinaryExpr : Expr, FullyVisitableExpr<BinaryExpr> {
public:
    TokenType op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(TokenType op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) :
        op{ op }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct GroupedExpr : Expr, FullyVisitableExpr<GroupedExpr> {
public:
    std::unique_ptr<Expr> expr;

    GroupedExpr(std::unique_ptr<Expr> expr) :
        expr{ std::move(expr) } {}
};
