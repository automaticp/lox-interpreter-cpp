#include "StmtVisitors.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "ExprVisitors.hpp"
#include <iostream>

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
    env_.define(stmt.identifier.lexeme, evaluate(*stmt.init));
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const BlockStmt& stmt) const {
    Environment block_env{ &env_ };
    StmtInterpreterVisitor block_visitor{ err_, block_env };

    for (const auto& statement : stmt.statements) {
        block_visitor.execute(*statement);
    }
}

StmtInterpreterVisitor::return_type
StmtInterpreterVisitor::operator()(const IfStmt& stmt) const {
    if (is_truthful(evaluate(*stmt.condition))) {
        execute(*stmt.then_branch);
    } else {
        execute(*stmt.else_branch);
    }
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
