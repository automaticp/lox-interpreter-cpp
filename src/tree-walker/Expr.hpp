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
#include "ExprVisitors.hpp"


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
