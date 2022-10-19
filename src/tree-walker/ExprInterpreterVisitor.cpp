#include "ExprVisitors.hpp"

#include "Expr.hpp"
#include "TokenType.hpp"
#include "Value.hpp"
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Environment.hpp"
#include <fmt/format.h>




ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::evaluate(const IExpr& expr) const {
    return expr.accept(*this);
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


void ExprInterpreterVisitor::report_error(InterpreterError type, const IExpr& expr, std::string_view details) const {
    err_.interpreter_error(type, expr, details);
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
            // FIXME: report failure?
            break;
    }
    return nullptr;
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


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const AssignExpr& expr) const {
    Value* val = env_.assign(
        expr.identifier.lexeme,
        evaluate(*expr.rvalue)
    );

    if (!val) {
        report_error_and_abort(InterpreterError::undefined_variable, expr, expr.identifier.lexeme);
    }
    return *val;
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const LogicalExpr& expr) const {
    Value lhs{ evaluate(*expr.lhs) };

    if (expr.op.type == TokenType::kw_or) {
        if (is_truthful(lhs)) { return lhs; }
    } else {
        if (!is_truthful(lhs)) { return lhs; }
    }

    return evaluate(*expr.rhs);
}

