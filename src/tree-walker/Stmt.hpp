#pragma once
#include "IStmt.hpp"
#include "IExpr.hpp"
#include "Token.hpp"
#include <memory>
#include <utility>
#include <vector>

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


struct BlockStmt : FullyVisitableStmt<BlockStmt> {
public:
    std::vector<std::unique_ptr<IStmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<IStmt>> statements) :
        statements{ std::move(statements) } {}
};


struct IfStmt : FullyVisitableStmt<IfStmt> {
public:
    std::unique_ptr<IExpr> condition;
    std::unique_ptr<IStmt> then_branch;
    std::unique_ptr<IStmt> else_branch;

    IfStmt(
        std::unique_ptr<IExpr> condition,
        std::unique_ptr<IStmt> then_branch,
        std::unique_ptr<IStmt> else_branch
    ) : condition{ std::move(condition) },
        then_branch{ std::move(then_branch) },
        else_branch{ std::move(else_branch) } {}
};


struct WhileStmt : FullyVisitableStmt<WhileStmt> {
public:
    std::unique_ptr<IExpr> condition;
    std::unique_ptr<IStmt> statement;

    WhileStmt(std::unique_ptr<IExpr> condition, std::unique_ptr<IStmt> statement) :
        condition{ std::move(condition) }, statement{ std::move(statement) } {}
};


struct FunStmt : FullyVisitableStmt<FunStmt> {
public:
    Token name;
    std::vector<Token> parameters;
    std::vector<std::unique_ptr<IStmt>> body;

    FunStmt(Token name, std::vector<Token> parameters, std::vector<std::unique_ptr<IStmt>> body) :
        name{ std::move(name) }, parameters{ std::move(parameters) }, body{ std::move(body) } {}
};


struct ReturnStmt : FullyVisitableStmt<ReturnStmt> {
public:
    Token keyword;
    std::unique_ptr<IExpr> expr;

    ReturnStmt(Token keyword, std::unique_ptr<IExpr> expr) :
        keyword{ std::move(keyword) }, expr{ std::move(expr) } {}
};