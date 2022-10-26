#include "StmtResolveVisitor.hpp"

#include "Errors.hpp"
#include "Stmt.hpp"
#include "Resolver.hpp"

void StmtResolveVisitor::resolve(const IStmt& stmt) const {
    stmt.accept(*this);
}

void StmtResolveVisitor::resolve_function(const FunStmt& stmt) const {
    resolver_.push_scope(ScopeType::function);
    for (const Token& param : stmt.parameters) {
        resolver_.declare(param.lexeme);
        resolver_.define(param.lexeme);
    }
    for (const auto& statement : stmt.body) {
        resolve(*statement);
    }
    resolver_.pop_scope();
}


bool StmtResolveVisitor::try_declare(const IStmt& stmt, const std::string& name) const {
    bool success{ resolver_.declare(name) };
    if (!success) {
        err_.resolver_error(
            ResolverError::local_variable_redeclaration, stmt,
            name
        );
    }
    return success;
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
    if (try_declare(stmt, stmt.identifier.lexeme)) {
        resolve(*stmt.init);
        resolver_.define(stmt.identifier.lexeme);
    }
}

StmtResolveVisitor::return_type
StmtResolveVisitor::operator()(const BlockStmt& stmt) const {
    resolver_.push_scope(ScopeType::block);
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
    if (!resolver_.is_in_function()) {
        err_.resolver_error(
            ResolverError::return_from_global_scope,
            stmt, ""
        );
    }

    resolve(*stmt.expr);
}
