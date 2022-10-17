#include "ExprVisitors.hpp"

#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "TokenType.hpp"
#include "Expr.hpp"
#include "Value.hpp"
#include <cstddef>
#include <variant>




// ---- ExprASTPrinterVisitor ----

ExprASTPrinterVisitor::return_type
ExprASTPrinterVisitor::operator()(const LiteralExpr& expr) const {
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





// ---- ExprInterpreterVisitor ----


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::evaluate(const IExpr& expr) const {
    return expr.accept(*this);
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const LiteralExpr& expr) const {
    return { std::visit([](auto&& arg) { return Value{ arg }; }, expr.token.literal.value()) };
}

ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const UnaryExpr& expr) const {
    Value val{ evaluate(*expr.operand) };

    switch (expr.op) {
        case TokenType::minus:
            check_type<double>(expr, val);
            return -std::get<double>(val);
            break;
        case TokenType::plus:
            check_type<double>(expr, val);
            break;
        case TokenType::bang:
            return !is_truthful(val);
            break;
        default:
            // FIXME: report failure
            break;
    }
    return nullptr;
}


bool ExprInterpreterVisitor::is_truthful(const Value& value) {
    if (holds<std::nullptr_t>(value)) {
        return false;
    }

    if (holds<bool>(value)) {
        return std::get<bool>(value);
    } else {
        return true;
    }
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const BinaryExpr& expr) const {
    Value lhs{ evaluate(*expr.lhs) };
    Value rhs{ evaluate(*expr.rhs) };

    using enum TokenType;
    switch (expr.op) {
        case minus:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) - std::get<double>(rhs);
        case slash:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) / std::get<double>(rhs);
        case star:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) * std::get<double>(rhs);
        case plus:
            if (holds<double>(lhs) && holds<double>(rhs)) {
                return std::get<double>(lhs) + std::get<double>(rhs);
            } else if (holds<std::string>(lhs) && holds<std::string>(rhs)) {
                return std::get<std::string>(lhs) + std::get<std::string>(rhs);
            } else {
                report_error(
                    InterpreterError::unexpected_type, expr,
                    fmt::format(
                        "Expected a pair of Numbers or Strings, Encountered {:s} and {:s}",
                        type_name(lhs), type_name(rhs)
                    )
                );
                abort_by_exception(InterpreterError::unexpected_type);
            }
            break;
        case greater:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) > std::get<double>(rhs);
        case greater_eq:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) >= std::get<double>(rhs);
        case less:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) < std::get<double>(rhs);
        case less_eq:
            check_type<double, double>(expr, lhs, rhs);
            return std::get<double>(lhs) <= std::get<double>(rhs);
        case eq_eq:
            return is_equal(lhs, rhs);
        case bang_eq:
            return !is_equal(lhs, rhs);
        default:
            break;
    }

    return nullptr;
}

ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const GroupedExpr& expr) const {
    return evaluate(*expr.expr);
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const VariableExpr& expr) const {
    Value* val = env_.get(expr.identifier.lexeme);
    if (!val) {
        report_error_and_abort(InterpreterError::undefined_variable, expr, expr.identifier.lexeme);
    }
    return *val;
}


void ExprInterpreterVisitor::report_error(InterpreterError type, const IExpr& expr, std::string_view details) const {
    err_.interpreter_error(type, expr, details);
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




// ---- ExprUserFriendlyNameVisitor ----

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const LiteralExpr& expr) const {
    return "Literal Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const UnaryExpr& expr) const {
    return "Unary Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const BinaryExpr& expr) const {
    return "Binary Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const GroupedExpr& expr) const {
    return "Group Expression";
}

ExprUserFriendlyNameVisitor::return_type
ExprUserFriendlyNameVisitor::operator()(const VariableExpr& expr) const {
    return "Variable Expression";
}

