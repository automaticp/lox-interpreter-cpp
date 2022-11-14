#include "CodegenVisitor.hpp"
#include "CommonVisitors.hpp"
#include "TokenType.hpp"
#include <fmt/format.h>
#include <variant>


// Helper to signal that certain nodes of the AST are not yet implemented
void CodegenVisitor::not_implemented(const Expr& expr) const {
    send_error(
        fmt::format(
            "[Error @Codegen]: Not implemented - {}\n",
            expr.accept(UserFriendlyNameVisitor{})
        )
    );
}

void CodegenVisitor::not_implemented(const Stmt& stmt) const {
    send_error(
        fmt::format(
            "[Error @Codegen]: Not implemented - {}\n",
            stmt.accept(UserFriendlyNameVisitor{})
        )
    );
}



void CodegenVisitor::operator()(const LiteralExpr& expr) const {
    assert(
        std::holds_alternative<Number>(expr.token.literal()) &&
        "Only Numbers are supported so far :("
    );
    chunk().emit_constant(std::get<Number>(expr.token.literal()));
}

void CodegenVisitor::operator()(const UnaryExpr& expr) const {
    assert(expr.op.type() == TokenType::minus);
    codegen(*expr.operand);
    chunk().emit(OP::NEGATE);
}

void CodegenVisitor::operator()(const BinaryExpr& expr) const {
    // Is reverse order correct?
    codegen(*expr.rhs);
    codegen(*expr.lhs);

    switch (expr.op.type()) {
        case TokenType::plus:
            chunk().emit(OP::ADD); break;
        case TokenType::minus:
            chunk().emit(OP::SUBTRACT); break;
        case TokenType::star:
            chunk().emit(OP::MULTIPLY); break;
        case TokenType::slash:
            chunk().emit(OP::DIVIDE); break;
        default:
            not_implemented(Expr::from_alternative(expr));
            break;
    }
}

void CodegenVisitor::operator()(const GroupedExpr& expr) const {
    codegen(*expr.expr);
}

void CodegenVisitor::operator()(const VariableExpr& expr) const {
    not_implemented(Expr::from_alternative(expr));
}

void CodegenVisitor::operator()(const AssignExpr& expr) const {
    not_implemented(Expr::from_alternative(expr));
}

void CodegenVisitor::operator()(const LogicalExpr& expr) const {
    not_implemented(Expr::from_alternative(expr));
}

void CodegenVisitor::operator()(const CallExpr& expr) const {
    not_implemented(Expr::from_alternative(expr));
}



void CodegenVisitor::operator()(const PrintStmt& stmt) const {
    codegen(*stmt.expr);
    chunk().emit(OP::PRINT);
}

void CodegenVisitor::operator()(const ExpressionStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const VarStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const BlockStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const IfStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const WhileStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const FunStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const ReturnStmt& stmt) const {
    not_implemented(Stmt::from_alternative(stmt));
}

void CodegenVisitor::operator()(const ImportStmt& stmt) const {
    // Do nothing?
}
