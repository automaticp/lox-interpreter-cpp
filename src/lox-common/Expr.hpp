#pragma once
#include <utility>
#include <memory>
#include <vector>
#include "Token.hpp"
#include "VariantWrapper.hpp"

class Expr;

using ExprBackref = WrapperBackreference<Expr>;



struct LiteralExpr : ExprBackref {
public:
    Token token;

    LiteralExpr(Token token) :
        token{ std::move(token) } {}
};


struct UnaryExpr : ExprBackref {
public:
    Token op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(Token op, std::unique_ptr<Expr> expr) :
        op{ std::move(op) }, operand{ std::move(expr) } {}
};


struct BinaryExpr : ExprBackref {
public:
    Token op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    BinaryExpr(Token op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) :
        op{ std::move(op) }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct GroupedExpr : ExprBackref {
public:
    std::unique_ptr<Expr> expr;

    GroupedExpr(std::unique_ptr<Expr> expr) :
        expr{ std::move(expr) } {}
};


struct VariableExpr : ExprBackref {
public:
    Token identifier;

    VariableExpr(Token identifier) :
        identifier{ std::move(identifier) } {}
};


struct AssignExpr : ExprBackref {
public:
    Token identifier;
    Token op;
    std::unique_ptr<Expr> rvalue;

    AssignExpr(Token identifier, Token op, std::unique_ptr<Expr> rvalue) :
        identifier{ std::move(identifier) }, op{ op }, rvalue{ std::move(rvalue) } {}

};


struct LogicalExpr : ExprBackref {
public:
    Token op;
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    LogicalExpr(Token op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) :
        op{ std::move(op) }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct CallExpr : ExprBackref {
public:
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    Token rparen;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> args, Token rparen) :
        callee{ std::move(callee) }, rparen{ std::move(rparen) }, args{ std::move(args) } {}
};





using ExprVariant = std::variant<
    LiteralExpr, UnaryExpr, BinaryExpr, GroupedExpr,
    VariableExpr, AssignExpr, LogicalExpr, CallExpr
>;


// ExprVariant wrapper
// FIXME: rename from IExpr to Expr
class Expr : public VariantWrapper<Expr, ExprVariant> {
public:
    using VariantWrapper<Expr, ExprVariant>::VariantWrapper;
    Expr() = delete;
};

