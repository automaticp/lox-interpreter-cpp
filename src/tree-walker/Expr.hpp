#pragma once
#include <variant>
#include <utility>
#include <memory>
#include "LiteralValue.hpp"
#include "Token.hpp"



struct ExprInterpretVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
};

struct ExprResolveVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
};

struct ExprAnalyzeVisitor {
    // FIXME: later
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
};


// If inherited from, the following classes
// add accept() overloads
// for all the visitors listed in ...Visitors
// with the correct return types

// IVisitableExpr adds pure vurtual interfaces

template<typename CRTP, typename ...Visitors>
struct IVisitableExpr;

template<typename CRTP, typename Visitor, typename ...OtherVisitors>
struct IVisitableExpr<CRTP, Visitor, OtherVisitors...> :
    IVisitableExpr<CRTP, OtherVisitors...> {

    virtual typename Visitor::return_type accept(Visitor& visitor) = 0;

};

template<typename CRTP>
struct IVisitableExpr<CRTP> {};


// VisitableExpr adds implementations

template<typename CRTP, typename ...Visitors>
struct VisitableExpr;

template<typename CRTP, typename Visitor, typename ...OtherVisitors>
struct VisitableExpr<CRTP, Visitor, OtherVisitors...> :
    VisitableExpr<CRTP, OtherVisitors...> {

    virtual typename Visitor::return_type accept(Visitor& visitor) {
        return visitor(static_cast<CRTP&>(*this));
    }

    friend Visitor;

};

template<typename CRTP>
struct VisitableExpr<CRTP> {};


// Extend these two lists to add more visitors
template<typename CRTP>
using FullyVisitableExpr = VisitableExpr<
    CRTP, ExprInterpretVisitor, ExprResolveVisitor, ExprAnalyzeVisitor
>;

template<typename CRTP>
using FullyVisitableExprInterface = IVisitableExpr<
    CRTP, ExprInterpretVisitor, ExprResolveVisitor, ExprAnalyzeVisitor
>;





struct Expr : FullyVisitableExprInterface<Expr> {
public:
    virtual ~Expr() = default;
};


struct LiteralExpr : Expr, FullyVisitableExpr<LiteralExpr> {
private:
    LiteralValue value_;
public:
    LiteralExpr(LiteralValue value) :
        value_{ std::move(value) } {}
};


struct UnaryExpr : Expr, FullyVisitableExpr<UnaryExpr> {
private:
    TokenType op_;
    std::unique_ptr<Expr> operand_;
public:
    UnaryExpr(TokenType op, std::unique_ptr<Expr> expr) :
        op_{ op }, operand_{ std::move(expr) } {}
};


struct BinaryExpr : Expr, FullyVisitableExpr<BinaryExpr> {
private:
    TokenType op_;
    std::unique_ptr<Expr> lhs_;
    std::unique_ptr<Expr> rhs_;
public:
    BinaryExpr(TokenType op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) :
        op_{ op }, lhs_{ std::move(lhs) }, rhs_{ std::move(rhs) } {}
};


struct GroupedExpr : Expr, FullyVisitableExpr<GroupedExpr> {
private:
    std::unique_ptr<Expr> expr_;
public:
    GroupedExpr(std::unique_ptr<Expr> expr) :
        expr_{ std::move(expr) } {}
};
