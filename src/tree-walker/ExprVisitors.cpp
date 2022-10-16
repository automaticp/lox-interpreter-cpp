#include "ExprVisitors.hpp"

#include "TokenType.hpp"
#include "Expr.hpp"
#include "Value.hpp"
#include <cstddef>
#include <variant>

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



ExprInterpretVisitor::return_type
ExprInterpretVisitor::evaluate(const IExpr& expr) const {
    return expr.accept(*this);
}


ExprInterpretVisitor::return_type
ExprInterpretVisitor::operator()(const LiteralExpr& expr) const {
    return { std::visit([](auto&& arg) { return Value{ arg }; }, expr.token.literal.value()) };
}

ExprInterpretVisitor::return_type
ExprInterpretVisitor::operator()(const UnaryExpr& expr) const {
    Value val{ evaluate(*expr.operand) };

    switch (expr.op) {
        case TokenType::minus:
            if (holds<double>(val)) {
                return -std::get<double>(val);
            } else {
                // FIXME: report failure
            }
            break;
        case TokenType::plus:
            if (!holds<double>(val)) {
                // FIXME: report failure
            }
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


bool ExprInterpretVisitor::is_truthful(const Value& value) {
    if (holds<std::nullptr_t>(value)) {
        return false;
    }

    if (holds<bool>(value)) {
        return std::get<bool>(value);
    } else {
        return true;
    }
}


ExprInterpretVisitor::return_type
ExprInterpretVisitor::operator()(const BinaryExpr& expr) const {
    Value lhs{ evaluate(*expr.lhs) };
    Value rhs{ evaluate(*expr.rhs) };

    using enum TokenType;
    switch (expr.op) {
        case minus:
            return try_cast<double>(lhs) - try_cast<double>(rhs);
        case slash:
            return try_cast<double>(lhs) / try_cast<double>(rhs);
        case star:
            return try_cast<double>(lhs) * try_cast<double>(rhs);
        case plus:
            if (holds<double>(lhs) && holds<double>(rhs)) {
                return std::get<double>(lhs) + std::get<double>(rhs);
            } else if (holds<std::string>(lhs) && holds<std::string>(rhs)) {
                return std::get<std::string>(lhs) + std::get<std::string>(rhs);
            } else {
                abort_by_exception(/* InterpreterError::unexpected_type */);
            }
            break;
        case greater:
            return try_cast<double>(lhs) > try_cast<double>(rhs);
        case greater_eq:
            return try_cast<double>(lhs) >= try_cast<double>(rhs);
        case less:
            return try_cast<double>(lhs) < try_cast<double>(rhs);
        case less_eq:
            return try_cast<double>(lhs) <= try_cast<double>(rhs);
        case eq_eq:
            return lhs == rhs;
        case bang_eq:
            return lhs != rhs;
        default:
            break;
    }

    return nullptr;
}

ExprInterpretVisitor::return_type
ExprInterpretVisitor::operator()(const GroupedExpr& expr) const {
    return evaluate(*expr.expr);
}
