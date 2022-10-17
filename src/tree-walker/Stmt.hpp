#pragma once
#include "IStmt.hpp"
#include "IExpr.hpp"
#include "Token.hpp"
#include <memory>
#include <utility>

struct ExpressionStmt : FullyVisitableStmt<ExpressionStmt> {
public:
    std::unique_ptr<IExpr> expr;

    ExpressionStmt(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}

};


struct PrintStmt : FullyVisitableStmt<PrintStmt> {
public:
    std::unique_ptr<IExpr> expr;

    PrintStmt(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}

};


struct VarStmt : FullyVisitableStmt<VarStmt> {
public:
    Token identifier;
    std::unique_ptr<IExpr> init;

    VarStmt(Token identifier, std::unique_ptr<IExpr> init) :
        identifier{ std::move(identifier) },
        init{ std::move(init) } {}
};
