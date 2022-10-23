#include "StmtVisitors.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "ExprVisitors.hpp"
#include "Token.hpp"
#include "Value.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <ranges>

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
    StmtInterpreterVisitor block_visitor{ err_, block_env, interpreter };

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

    env.define(
        stmt.name.lexeme,
        Function{
            &stmt,
            std::move(closure)
        }
    );
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const ReturnStmt& stmt) const {
    // Walk up the call stack with exceptions.
    // To be caught in the Function::operator()
    throw evaluate(*stmt.expr);
}



void StmtInterpreterVisitor::execute(const IStmt& stmt) const {
    stmt.accept(*this);
}







StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const PrintStmt& stmt) const {
    return fmt::format("print {:s};", stmt.expr->accept(*this));
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const ExpressionStmt& stmt) const {
    return fmt::format("{:s};", stmt.expr->accept(*this));
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const VarStmt& stmt) const {
    return fmt::format("var {:s} = {:s};", stmt.identifier.lexeme, stmt.init->accept(*this));
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const BlockStmt& stmt) const {
    std::string result{ "{\n" };
    for (const auto& statement : stmt.statements) {
        result += statement->accept(*this) + '\n';
    }
    result += "}";
    return result;
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const IfStmt& stmt) const {
    return fmt::format(
        "if ({}) {} {} {}",
        stmt.condition->accept(*this),
        stmt.then_branch->accept(*this),
        stmt.else_branch ? "else" : "",
        stmt.else_branch ? stmt.else_branch->accept(*this) : ""
    );
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const WhileStmt& stmt) const {
    return fmt::format(
        "while ({}) {}",
        stmt.condition->accept(*this),
        stmt.statement->accept(*this)
    );
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const FunStmt& stmt) const {
    // I wanted to do ranges but it seems I don't know ranges =(
    auto join_token_names = [](const std::vector<Token>& ts) -> std::string {
        if (ts.empty()) return { "" };

        std::string result;
        auto it{ ts.begin() };

        for (; it < ts.end() - 1; ++it) {
            result += it->lexeme + ", ";
        }
        result += (++it)->lexeme;

        return result;
    };

    auto join_statements = [this](const std::vector<std::unique_ptr<IStmt>>& stmts) -> std::string {
        std::string result;
        for (const auto& s : stmts) {
            result += s->accept(*this);
        }
        return result;
    };

    return fmt::format(
        "fun {}({}) {{\n{}}}\n",
        stmt.name.lexeme,
        join_token_names(stmt.parameters),
        join_statements(stmt.body)
    );
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const ReturnStmt& stmt) const {
    return fmt::format(
        "return {};", stmt.expr->accept(*this)
    );
}
