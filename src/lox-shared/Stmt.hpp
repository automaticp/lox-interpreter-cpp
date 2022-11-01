#pragma once
#include "Expr.hpp"
#include "Token.hpp"
#include "VariantWrapper.hpp"
#include <memory>
#include <utility>
#include <vector>


class IStmt;


using StmtBackref = WrapperBackreference<IStmt>;



struct ExpressionStmt : StmtBackref {
public:
    std::unique_ptr<IExpr> expr;

    ExpressionStmt(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}
};



struct PrintStmt : StmtBackref {
public:
    std::unique_ptr<IExpr> expr;

    PrintStmt(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}
};


struct VarStmt : StmtBackref {
public:
    Token identifier;
    std::unique_ptr<IExpr> init;

    VarStmt(Token identifier, std::unique_ptr<IExpr> init) :
        identifier{ std::move(identifier) },
        init{ std::move(init) } {}
};


struct BlockStmt : StmtBackref {
public:
    std::vector<std::unique_ptr<IStmt>> statements;

    BlockStmt(std::vector<std::unique_ptr<IStmt>> statements) :
        statements{ std::move(statements) } {}
};


struct IfStmt : StmtBackref {
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


struct WhileStmt : StmtBackref {
public:
    std::unique_ptr<IExpr> condition;
    std::unique_ptr<IStmt> statement;

    WhileStmt(std::unique_ptr<IExpr> condition, std::unique_ptr<IStmt> statement) :
        condition{ std::move(condition) }, statement{ std::move(statement) } {}
};


struct FunStmt : StmtBackref {
public:
    Token name;
    std::vector<Token> parameters;
    std::vector<std::unique_ptr<IStmt>> body;

    FunStmt(Token name, std::vector<Token> parameters, std::vector<std::unique_ptr<IStmt>> body) :
        name{ std::move(name) }, parameters{ std::move(parameters) }, body{ std::move(body) } {}
};


struct ReturnStmt : StmtBackref {
public:
    Token keyword;
    std::unique_ptr<IExpr> expr;

    ReturnStmt(Token keyword, std::unique_ptr<IExpr> expr) :
        keyword{ std::move(keyword) }, expr{ std::move(expr) } {}
};






using StmtVariant = std::variant<
    ExpressionStmt, PrintStmt, VarStmt, BlockStmt,
    IfStmt, WhileStmt, FunStmt, ReturnStmt
>;



// StmtVariant wrapper
// FIXME: rename from IStmt to Stmt
class IStmt : public VariantWrapper<IStmt, StmtVariant> {
public:
    using VariantWrapper<IStmt, StmtVariant>::VariantWrapper;
    IStmt() = delete;
};

