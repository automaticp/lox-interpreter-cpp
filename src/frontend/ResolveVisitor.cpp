#include "ResolveVisitor.hpp"

#include "Resolver.hpp"
#include "CommonVisitors.hpp"
#include <algorithm>


// Private member functions

void ResolveVisitor::resolve(const Expr& expr) const {
    expr.accept(*this);
}

void ResolveVisitor::resolve(const Stmt& stmt) const {
    stmt.accept(*this);
}

// I'm tired, forgive me for the next 2 functions

size_t ResolveVisitor::distance_to_enclosing_fun_scope() const {
    size_t num_scopes{ resolver_.scopes().size() };

    size_t i{ 0 };
    for (; i < num_scopes; ++i) {
        if (resolver_.scope_types()[num_scopes - 1 - i] == ScopeType::function) {
            return i;
        }
    }
    return i; // i == num_scopes
}

size_t ResolveVisitor::distance_to_var_decl(const std::string& name) const {
    size_t num_scopes{ resolver_.scopes().size() };

    size_t i{ 0 };
    for (; i < num_scopes; ++i) {
        if (resolver_.scope_at(num_scopes - 1 - i).contains(name)) {
            return i;
        }
    }
    return i; // i == num_scopes
}


void ResolveVisitor::resolve_local(const Expr& expr, const std::string& name) const {

    size_t lexical_distance{ distance_to_var_decl(name) };

    if (lexical_distance < resolver_.scopes().size()) { // Validate that it's declared at all

        size_t would_be_closure_distance{ distance_to_enclosing_fun_scope() + 1 };

        // If we're within the scope of the same function, then it's local to it's body.
        // Otherwise, look in the closure of that fucntion.
        size_t real_distance = std::min(lexical_distance, would_be_closure_distance);
        resolver_.set_depth(expr, real_distance);

    } else /* not resolved */ {

        resolver_.send_error(
            ResolverError::Type::undefined_variable,
            expr.accept(ExprGetPrimaryTokenVisitor{}),
            name_of(expr),
            name
        );
    }
}



void ResolveVisitor::resolve_function(const FunStmt& stmt) const {
    resolver_.push_scope(ScopeType::function);
    for (const Token& param : stmt.parameters) {
        resolver_.declare(param.lexeme());
        resolver_.define(param.lexeme());
    }
    for (const auto& statement : stmt.body) {
        resolve(*statement);
    }
    resolver_.pop_scope();
}

bool ResolveVisitor::try_declare(const VarStmt& stmt, const std::string& name) const {
    bool success{ resolver_.declare(name) };
    if (!success) {
        resolver_.send_error(
            ResolverError::Type::local_variable_redeclaration,
            stmt.identifier,
            name_of(Stmt::from_alternative(stmt)),
            name
        );
    }
    return success;
}






// Expr visitor overloads

void ResolveVisitor::operator()(const LiteralExpr&) const {
    return;
}

void ResolveVisitor::operator()(const UnaryExpr& expr) const {
    resolve(*expr.operand);
}


void ResolveVisitor::operator()(const BinaryExpr& expr) const {
    resolve(*expr.lhs);
    resolve(*expr.rhs);
}

void ResolveVisitor::operator()(const GroupedExpr& expr) const {
    resolve(*expr.expr);
}

void ResolveVisitor::operator()(const VariableExpr& expr) const {

    if (!resolver_.is_in_global_scope()) {
        auto it = resolver_.top_scope().find(expr.identifier.lexeme());
        if (it != resolver_.top_scope().end() && it->second == ResolveState::declared) {
            resolver_.send_error(
                ResolverError::Type::initialization_from_self,
                expr.identifier,
                name_of(Expr::from_alternative(expr)),
                ""
            );
        }
    }

    resolve_local(expr, expr.identifier.lexeme());
}

void ResolveVisitor::operator()(const AssignExpr& expr) const {
    resolve(*expr.rvalue);
    resolve_local(expr, expr.identifier.lexeme());
}

void ResolveVisitor::operator()(const LogicalExpr& expr) const {
    resolve(*expr.lhs);
    resolve(*expr.rhs);
}

void ResolveVisitor::operator()(const CallExpr& expr) const {
    resolve(*expr.callee);
    for (const auto& arg : expr.args) {
        resolve(*arg);
    }
}




// Stmt visitor overloads


void ResolveVisitor::operator()(const PrintStmt& stmt) const {
    resolve(*stmt.expr);
}

void ResolveVisitor::operator()(const ExpressionStmt& stmt) const {
    resolve(*stmt.expr);
}

void ResolveVisitor::operator()(const VarStmt& stmt) const {
    if (try_declare(stmt, stmt.identifier.lexeme())) {
        resolve(*stmt.init);
        resolver_.define(stmt.identifier.lexeme());
    }
}

void ResolveVisitor::operator()(const BlockStmt& stmt) const {
    resolver_.push_scope(ScopeType::block);
    for (const auto& statement : stmt.statements) {
        resolve(*statement);
    }
    resolver_.pop_scope();
}

void ResolveVisitor::operator()(const IfStmt& stmt) const {
    resolve(*stmt.condition);
    resolve(*stmt.then_branch);
    if (stmt.else_branch) resolve(*stmt.else_branch);
}

void ResolveVisitor::operator()(const WhileStmt& stmt) const {
    resolve(*stmt.condition);
    resolve(*stmt.statement);
}

void ResolveVisitor::operator()(const FunStmt& stmt) const {
    resolver_.declare(stmt.name.lexeme());
    resolver_.define(stmt.name.lexeme());
    resolve_function(stmt);
}

void ResolveVisitor::operator()(const ReturnStmt& stmt) const {
    if (!resolver_.is_in_function()) {
        resolver_.send_error(
            ResolverError::Type::return_from_global_scope,
            stmt.keyword,
            name_of(Stmt::from_alternative(stmt)),
            ""
        );
    }

    resolve(*stmt.expr);
}
