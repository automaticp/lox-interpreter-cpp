#pragma once
#include <string>

class IExpr;
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

    ExprResolveVisitor(Resolver& resolver, Interpreter& interpreter, ErrorReporter& err) :
        resolver_{ resolver }, interpreter_{ interpreter }, err_{ err } {}
protected:
    Interpreter& interpreter_;
    Resolver& resolver_;
    ErrorReporter& err_;

protected:
    void resolve(const IExpr& expr) const;
    void resolve_local(const IExpr& expr, const std::string& name) const;
};

