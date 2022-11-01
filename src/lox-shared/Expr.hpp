#pragma once
#include <utility>
#include <memory>
#include <vector>
#include "Token.hpp"
#include "VariantWrapper.hpp"

class IExpr;

using ExprBackref = WrapperBackreference<IExpr>;



struct LiteralExpr : ExprBackref {
public:
    Token token;

    LiteralExpr(Token token) :
        token{ std::move(token) } {}
};


struct UnaryExpr : ExprBackref {
public:
    Token op;
    std::unique_ptr<IExpr> operand;

    UnaryExpr(Token op, std::unique_ptr<IExpr> expr) :
        op{ std::move(op) }, operand{ std::move(expr) } {}
};


struct BinaryExpr : ExprBackref {
public:
    Token op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;

    BinaryExpr(Token op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs) :
        op{ std::move(op) }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct GroupedExpr : ExprBackref {
public:
    std::unique_ptr<IExpr> expr;

    GroupedExpr(std::unique_ptr<IExpr> expr) :
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
    std::unique_ptr<IExpr> rvalue;

    AssignExpr(Token identifier, Token op, std::unique_ptr<IExpr> rvalue) :
        identifier{ std::move(identifier) }, op{ op }, rvalue{ std::move(rvalue) } {}

};


struct LogicalExpr : ExprBackref {
public:
    Token op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;

    LogicalExpr(Token op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs) :
        op{ std::move(op) }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct CallExpr : ExprBackref {
public:
    std::unique_ptr<IExpr> callee;
    std::vector<std::unique_ptr<IExpr>> args;
    Token rparen;

    CallExpr(std::unique_ptr<IExpr> callee, std::vector<std::unique_ptr<IExpr>> args, Token rparen) :
        callee{ std::move(callee) }, rparen{ std::move(rparen) }, args{ std::move(args) } {}
};





using ExprVariant = std::variant<
    LiteralExpr, UnaryExpr, BinaryExpr, GroupedExpr,
    VariableExpr, AssignExpr, LogicalExpr, CallExpr
>;


// ExprVariant wrapper
// FIXME: rename from IExpr to Expr
class IExpr : public VariantWrapper<IExpr, ExprVariant> {
public:
    using VariantWrapper<IExpr, ExprVariant>::VariantWrapper;
    IExpr() = delete;
};

