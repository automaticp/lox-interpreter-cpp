#pragma once
#include "ExprResolveVisitor.hpp"


class Stmt;
class PrintStmt;
class ExpressionStmt;
class VarStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;
class FunStmt;

class Resolver;

struct StmtResolveVisitor : protected ExprResolveVisitor {
public:
    using return_type = void;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
    return_type operator()(const VarStmt& stmt) const;
    return_type operator()(const BlockStmt& stmt) const;
    return_type operator()(const IfStmt& stmt) const;
    return_type operator()(const WhileStmt& stmt) const;
    return_type operator()(const FunStmt& stmt) const;
    return_type operator()(const ReturnStmt& stmt) const;

    StmtResolveVisitor(Resolver& resolver) :
        ExprResolveVisitor{ resolver } {}

private:
    using ExprResolveVisitor::resolve;
    void resolve(const Stmt& stmt) const;
    void resolve_function(const FunStmt& stmt) const;

    bool try_declare(const VarStmt& stmt, const std::string& name) const;
};

