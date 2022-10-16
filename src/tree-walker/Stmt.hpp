#pragma once
#include "IStmt.hpp"
#include "IExpr.hpp"
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
