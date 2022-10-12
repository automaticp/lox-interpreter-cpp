#include "ExprVisitors.hpp"

#include "TokenType.hpp"
#include "Expr.hpp"

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const LiteralExpr& expr) const {
    return to_string(expr.value);
}

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const UnaryExpr& expr) const {
    return parenthesize(to_lexeme(expr.op), *expr.operand);
}

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const BinaryExpr& expr) const {
    return parenthesize(to_lexeme(expr.op), *expr.lhs, *expr.rhs);
}

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const GroupedExpr& expr) const {
    return parenthesize("group", *expr.expr);
}
