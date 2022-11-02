#pragma once
#include <string>

class Expr;
class LiteralExpr;
class UnaryExpr;
class BinaryExpr;
class GroupedExpr;
class VariableExpr;
class AssignExpr;
class LogicalExpr;
class CallExpr;

class Resolver;
class Interpreter;
class ErrorReporter;


struct ExprResolveVisitor {
public:
    using return_type = void;
    return_type operator()(const LiteralExpr& expr) const;
    return_type operator()(const UnaryExpr& expr) const;
    return_type operator()(const BinaryExpr& expr) const;
    return_type operator()(const GroupedExpr& expr) const;
    return_type operator()(const VariableExpr& expr) const;
    return_type operator()(const AssignExpr& expr) const;
    return_type operator()(const LogicalExpr& expr) const;
    return_type operator()(const CallExpr& expr) const;

    ExprResolveVisitor(Resolver& resolver) :
        resolver_{ resolver } {}
protected:
    Resolver& resolver_;

protected:
    void resolve(const Expr& expr) const;
    void resolve_local(const Expr& expr, const std::string& name) const;
};

