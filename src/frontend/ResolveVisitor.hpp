#pragma once
#include "Expr.hpp"
#include "Stmt.hpp"




class Resolver;

class ResolveVisitor {
private:
    Resolver& resolver_;

public:
    ResolveVisitor(Resolver& resolver) : resolver_{ resolver } {}


    // Expr overloads

    void operator()(const LiteralExpr& expr) const;
    void operator()(const UnaryExpr& expr) const;
    void operator()(const BinaryExpr& expr) const;
    void operator()(const GroupedExpr& expr) const;
    void operator()(const VariableExpr& expr) const;
    void operator()(const AssignExpr& expr) const;
    void operator()(const LogicalExpr& expr) const;
    void operator()(const CallExpr& expr) const;

    // Stmt overloads

    void operator()(const PrintStmt& stmt) const;
    void operator()(const ExpressionStmt& stmt) const;
    void operator()(const VarStmt& stmt) const;
    void operator()(const BlockStmt& stmt) const;
    void operator()(const IfStmt& stmt) const;
    void operator()(const WhileStmt& stmt) const;
    void operator()(const FunStmt& stmt) const;
    void operator()(const ReturnStmt& stmt) const;
    void operator()(const ImportStmt& stmt) const;


private:
    void resolve(const Expr& expr) const;
    void resolve_local(const Expr& expr, const std::string& name) const;

    void resolve(const Stmt& stmt) const;
    void resolve_function(const FunStmt& stmt) const;

    bool try_declare(const VarStmt& stmt, const std::string& name) const;

    size_t distance_to_var_decl(const std::string& name) const;
    size_t distance_to_enclosing_fun_scope() const;

};
