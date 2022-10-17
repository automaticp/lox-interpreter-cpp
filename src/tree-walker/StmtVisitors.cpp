#include "StmtVisitors.hpp"
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
