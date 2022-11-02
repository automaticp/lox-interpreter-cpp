#include "StmtInterpreterVisitor.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "ExprVisitors.hpp"
#include "Token.hpp"
#include "Value.hpp"



StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const PrintStmt& stmt) const {
    auto value = evaluate(*stmt.expr);
    std::cout << to_string(value) << '\n';
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const ExpressionStmt& stmt) const {
    evaluate(*stmt.expr);
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const VarStmt& stmt) const {
    env.define(stmt.identifier.lexeme, evaluate(*stmt.init));
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const BlockStmt& stmt) const {
    Environment block_env{ &env };
    StmtInterpreterVisitor block_visitor{ block_env, interpreter };

    for (const auto& statement : stmt.statements) {
        block_visitor.execute(*statement);
    }
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const IfStmt& stmt) const {
    if (is_truthful(evaluate(*stmt.condition))) {
        execute(*stmt.then_branch);
    } else if (stmt.else_branch) {
        execute(*stmt.else_branch);
    }
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const WhileStmt& stmt) const {
    while (is_truthful(evaluate(*stmt.condition))) {
        execute(*stmt.statement);
    }
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const FunStmt& stmt) const {
    // Hail Mary closure that copies EVERYTHING from outer scopes,
    // essentially, storing the state of the entire program at capture time.
    // Absolutely horrible, but should work.
    //
    // First, intialize with the copy of the current scope.
    Environment closure{ nullptr, env.map() };

    // Ther recursively copy values for symbols not yet in closure,
    // starting from the inner-most enclosing scope.
    Environment* enclosing{ env.enclosing() };
    while (enclosing) {
        for (const auto& elem : enclosing->map()) {
            if (!closure.get(elem.first)) {
                closure.define(elem.first, elem.second);
            }
        }
        enclosing = enclosing->enclosing();
    }

    // Add this function to the current environment.
    ValueHandle fun_handle = env.define(
        stmt.name.lexeme,
        Function{
            &stmt,
            std::move(closure)
        }
    );

    // Also add a copy? of itself to the closure.
    fun_handle.unwrap_to<Function>().closure().define(stmt.name.lexeme, fun_handle.decay());

}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const ReturnStmt& stmt) const {
    // Walk up the call stack with exceptions.
    // To be caught in the Function::operator()
    throw evaluate(*stmt.expr);
}



void StmtInterpreterVisitor::execute(const Stmt& stmt) const {
    stmt.accept(*this);
}



