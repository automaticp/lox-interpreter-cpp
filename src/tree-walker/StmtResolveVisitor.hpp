#pragma once
#include "ExprResolveVisitor.hpp"


class IStmt;
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

    StmtResolveVisitor(Resolver& resolver, ErrorReporter& err) :
        ExprResolveVisitor(resolver, err) {}

private:
    using ExprResolveVisitor::resolve;
    void resolve(const IStmt& stmt) const;
    void resolve_function(const FunStmt& stmt) const;

    bool try_declare(const IStmt& stmt, const std::string& name) const;
};

