#include "StmtResolveVisitor.hpp"

#include "Stmt.hpp"
#include "Resolver.hpp"

void StmtResolveVisitor::resolve(const IStmt& stmt) const {
    stmt.accept(*this);
}

void StmtResolveVisitor::resolve_function(const FunStmt& stmt) const {
    resolver_.push_scope();
    for (const Token& param : stmt.parameters) {
        resolver_.declare(param.lexeme);
        resolver_.define(param.lexeme);
    }
    for (const auto& statement : stmt.body) {
        resolve(*statement);
    }
    resolver_.pop_scope();
}




StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const PrintStmt& stmt) const {
    resolve(*stmt.expr);
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const ExpressionStmt& stmt) const {
    resolve(*stmt.expr);
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const VarStmt& stmt) const {
    resolver_.declare(stmt.identifier.lexeme);
    resolve(*stmt.init);
    resolver_.define(stmt.identifier.lexeme);
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const BlockStmt& stmt) const {
    resolver_.push_scope();
    for (const auto& statement : stmt.statements) {
        resolve(*statement);
    }
    resolver_.pop_scope();
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const IfStmt& stmt) const {
    resolve(*stmt.condition);
    resolve(*stmt.then_branch);
    if (stmt.else_branch) resolve(*stmt.else_branch);
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const WhileStmt& stmt) const {
    resolve(*stmt.condition);
    resolve(*stmt.statement);
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const FunStmt& stmt) const {
    resolver_.declare(stmt.name.lexeme);
    resolver_.define(stmt.name.lexeme);
    resolve_function(stmt);
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const ReturnStmt& stmt) const {
    resolve(*stmt.expr);
}
