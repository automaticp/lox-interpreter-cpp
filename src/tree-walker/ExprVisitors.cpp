#include "ExprVisitors.hpp"

#include "TokenType.hpp"
#include "Expr.hpp"
#include <cstddef>
#include <variant>




// ---- ExprASTPrinterVisitor ----

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const LiteralExpr& expr) const {
    assert(expr.token.literal);
    return to_string(expr.token.literal.value());
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

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const VariableExpr& expr) const {
    return expr.identifier.lexeme;
}

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const AssignExpr& expr) const {
    return parenthesize(expr.identifier.lexeme, *expr.rvalue);
}

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const LogicalExpr& expr) const {
    return parenthesize(to_lexeme(expr.op), *expr.lhs, *expr.rhs);
}

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const CallExpr& expr) const {
    return call_expr_string(expr);
}



std::string ExprASTPrinterVisitor::call_expr_string(const CallExpr& expr) const {
    std::string result{ "(call " };
    result += expr.callee->accept(*this) + " ";
    for (const auto& arg : expr.args) {
        result += arg->accept(*this) + " ";
    }
    result += ")";
    return result;
}




// ---- ExprGetPrimaryTokenVisitor ----


ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const LiteralExpr& expr) const {
    return expr.token;
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const UnaryExpr& expr) const {
    return expr.op;
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const BinaryExpr& expr) const {
    return expr.op;
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const GroupedExpr& expr) const {
    return expr.expr->accept(*this);
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const VariableExpr& expr) const {
    return expr.identifier;
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const AssignExpr& expr) const {
    return expr.op;
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const LogicalExpr& expr) const {
    return expr.op;
}

ExprGetPrimaryTokenVisitor::return_type
ExprGetPrimaryTokenVisitor::operator()(const CallExpr& expr) const {
    return expr.rparen;
}



// ---- ExprUserFriendlyNameVisitor ----

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const LiteralExpr&) const {
    return "Literal Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const UnaryExpr&) const {
    return "Unary Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const BinaryExpr&) const {
    return "Binary Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const GroupedExpr&) const {
    return "Group Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const VariableExpr&) const {
    return "Variable Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const AssignExpr&) const {
    return "Assignment Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const LogicalExpr&) const {
    return "Logical Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const CallExpr&) const {
    return "Call Expression";
}
