#pragma once
#include <utility>
#include <memory>
#include "LiteralValue.hpp"
#include "TokenType.hpp"
#include "IExpr.hpp"



struct LiteralExpr : FullyVisitableExpr<LiteralExpr> {
public:
    LiteralValue value;

    LiteralExpr(LiteralValue value) :
        value{ std::move(value) } {}
};


struct UnaryExpr : FullyVisitableExpr<UnaryExpr> {
public:
    TokenType op;
    std::unique_ptr<IExpr> operand;

    UnaryExpr(TokenType op, std::unique_ptr<IExpr> expr) :
        op{ op }, operand{ std::move(expr) } {}
};


struct BinaryExpr : FullyVisitableExpr<BinaryExpr> {
public:
    TokenType op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;

    BinaryExpr(TokenType op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs) :
        op{ op }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct GroupedExpr : FullyVisitableExpr<GroupedExpr> {
public:
    std::unique_ptr<IExpr> expr;

    GroupedExpr(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}
};
