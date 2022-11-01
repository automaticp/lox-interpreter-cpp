#pragma once
#include "Expr.hpp"
#include "Token.hpp"
#include "VariantWrapper.hpp"
#include <memory>
#include <utility>
#include <vector>


class Stmt;


using StmtBackref = WrapperBackreference<Stmt>;



struct ExpressionStmt : StmtBackref {
public:
    std::unique_ptr<Expr> expr;

    ExpressionStmt(std::unique_ptr<Expr> expr) :
        expr{ std::move(expr) } {}
};



struct PrintStmt : StmtBackref {
public:
    std::unique_ptr<Expr> expr;

    PrintStmt(std::unique_ptr<Expr> expr) :
        expr{ std::move(expr) } {}
};


struct VarStmt : StmtBackref {
public:
    Token identifier;
    std::unique_ptr<Expr> init;

    VarStmt(Token identifier, std::unique_ptr<Expr> init) :
        identifier{ std::move(identifier) },
        init{ std::move(init) } {}
};


struct BlockStmt : StmtBackref {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements) :
        statements{ std::move(statements) } {}
};


struct IfStmt : StmtBackref {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;

    IfStmt(
        std::unique_ptr<Expr> condition,
        std::unique_ptr<Stmt> then_branch,
        std::unique_ptr<Stmt> else_branch
    ) : condition{ std::move(condition) },
        then_branch{ std::move(then_branch) },
        else_branch{ std::move(else_branch) } {}
};


struct WhileStmt : StmtBackref {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> statement;

    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> statement) :
        condition{ std::move(condition) }, statement{ std::move(statement) } {}
};


struct FunStmt : StmtBackref {
public:
    Token name;
    std::vector<Token> parameters;
    std::vector<std::unique_ptr<Stmt>> body;

    FunStmt(Token name, std::vector<Token> parameters, std::vector<std::unique_ptr<Stmt>> body) :
        name{ std::move(name) }, parameters{ std::move(parameters) }, body{ std::move(body) } {}
};


struct ReturnStmt : StmtBackref {
public:
    Token keyword;
    std::unique_ptr<Expr> expr;

    ReturnStmt(Token keyword, std::unique_ptr<Expr> expr) :
        keyword{ std::move(keyword) }, expr{ std::move(expr) } {}
};






using StmtVariant = std::variant<
    ExpressionStmt, PrintStmt, VarStmt, BlockStmt,
    IfStmt, WhileStmt, FunStmt, ReturnStmt
>;



// StmtVariant wrapper
// FIXME: rename from IStmt to Stmt
class Stmt : public VariantWrapper<Stmt, StmtVariant> {
public:
    using VariantWrapper<Stmt, StmtVariant>::VariantWrapper;
    Stmt() = delete;
};

