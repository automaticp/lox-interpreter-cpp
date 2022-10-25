#include "ExprResolveVisitor.hpp"
#include "ExprVisitors.hpp"

#include "Interpreter.hpp"
#include "Expr.hpp"
#include "Resolver.hpp"

void ExprResolveVisitor::resolve(const IExpr& expr) const {
    expr.accept(*this);
}

void ExprResolveVisitor::resolve_local(const IExpr& expr, const std::string& name) const {
    size_t num_scopes{ resolver_.scopes().size() };

    for (size_t i{ num_scopes - 1}; i >= 0; --i) {
        if (resolver_.scope_at(i).contains(name)) {
            resolver_.set_depth(expr, num_scopes - 1 - i);
            return;
        }
    }
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const LiteralExpr& /* expr */) const {
    return;
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const UnaryExpr& expr) const {
    resolve(*expr.operand);
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const BinaryExpr& expr) const {
    resolve(*expr.lhs);
    resolve(*expr.rhs);
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const GroupedExpr& expr) const {
    resolve(*expr.expr);
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const VariableExpr& expr) const {

    if (!resolver_.is_in_global_scope()) {
        auto it = resolver_.top_scope().find(expr.identifier.lexeme);
        if (it != resolver_.top_scope().end() && it->second == ResolveState::declared) {
            err_.resolver_error(
                ResolverError::initialization_from_self,
                expr, ""
            );
        }
    }

    resolve_local(expr, expr.identifier.lexeme);
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const AssignExpr& expr) const {
    resolve(*expr.rvalue);
    resolve_local(expr, expr.identifier.lexeme);
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const LogicalExpr& expr) const {
    resolve(*expr.lhs);
    resolve(*expr.rhs);
}

ExprResolveVisitor::return_type
ExprResolveVisitor::operator()(const CallExpr& expr) const {
    resolve(*expr.callee);
    for (const auto& arg : expr.args) {
        resolve(*arg);
    }
}

