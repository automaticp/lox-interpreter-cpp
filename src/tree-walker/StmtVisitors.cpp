#include "StmtVisitors.hpp"
#include "Stmt.hpp"
#include "ExprVisitors.hpp"
#include <iostream>

StmtEvaluateVisitor::return_type
StmtEvaluateVisitor::operator()(const PrintStmt& stmt) const {
    auto value = stmt.expr->accept(ExprInterpreterVisitor{ err_ });
    std::cout << to_string(value) << '\n';
}

StmtEvaluateVisitor::return_type
StmtEvaluateVisitor::operator()(const ExpressionStmt& stmt) const {
    stmt.expr->accept(ExprInterpreterVisitor{ err_ });
}
