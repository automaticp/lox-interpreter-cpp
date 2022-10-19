#pragma once
#include <utility>
#include <memory>
#include "LiteralValue.hpp"
#include "TokenType.hpp"
#include "IExpr.hpp"
#include "Token.hpp"


struct LiteralExpr : FullyVisitableExpr<LiteralExpr> {
public:
    Token token;

    LiteralExpr(Token token) :
        token{ std::move(token) } {}
};


struct UnaryExpr : FullyVisitableExpr<UnaryExpr> {
public:
    Token op;
    std::unique_ptr<IExpr> operand;

    UnaryExpr(Token op, std::unique_ptr<IExpr> expr) :
        op{ std::move(op) }, operand{ std::move(expr) } {}
};


struct BinaryExpr : FullyVisitableExpr<BinaryExpr> {
public:
    Token op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;

    BinaryExpr(Token op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs) :
        op{ std::move(op) }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct GroupedExpr : FullyVisitableExpr<GroupedExpr> {
public:
    std::unique_ptr<IExpr> expr;

    GroupedExpr(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}
};


struct VariableExpr : FullyVisitableExpr<VariableExpr> {
public:
    Token identifier;

    VariableExpr(Token identifier) :
        identifier{ std::move(identifier) } {}
};


struct AssignExpr : FullyVisitableExpr<AssignExpr> {
public:
    Token identifier;
    Token op;
    std::unique_ptr<IExpr> rvalue;

    AssignExpr(Token identifier, Token op, std::unique_ptr<IExpr> rvalue) :
        identifier{ std::move(identifier) }, op{ op }, rvalue{ std::move(rvalue) } {}

};


struct LogicalExpr : FullyVisitableExpr<LogicalExpr> {
    Token op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;

    LogicalExpr(Token op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs) :
        op{ std::move(op) }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};
