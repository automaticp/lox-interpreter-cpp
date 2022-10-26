#include "ExprVisitors.hpp"

#include "Expr.hpp"
#include "TokenType.hpp"
#include "Value.hpp"
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Environment.hpp"
#include "Interpreter.hpp"
#include <fmt/format.h>
#include <variant>




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
    err.interpreter_error(type, expr, details);
}






ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const LiteralExpr& expr) const {
    assert(expr.token.literal.has_value());
    return {
        std::visit(
            [](auto&& arg) { return Value{ arg }; },
            expr.token.literal.value()
        )
    };
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
            return lhs == rhs;
        case bang_eq:
            return lhs != rhs;
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

    // FIXME now that the global scope is proper scope
    auto& depths = interpreter.resolver_.depth_map();
    auto it = depths.find(&expr);
    if (it != depths.end()) {
        return *env.get_at(it->second, expr.identifier.lexeme);
    } else {
        Value* val = interpreter.env_.get(expr.identifier.lexeme);
        if (!val) {
            report_error_and_abort(
                InterpreterError::undefined_variable,
                expr, expr.identifier.lexeme
            );
        }
        return *val;
    }

}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const AssignExpr& expr) const {
    auto& depths = interpreter.resolver_.depth_map();

    auto it = depths.find(&expr);
    if (it != depths.end()) {
        Value* val = env.assign_at(
            it->second,
            expr.identifier.lexeme,
            evaluate(*expr.rvalue)
        );
        assert(val); // This whole thing is so fragile, I hate it
        return *val;
    } else {
        Value* val = interpreter.env_.assign(
            expr.identifier.lexeme,
            evaluate(*expr.rvalue)
        );

        if (!val) {
            report_error_and_abort(InterpreterError::undefined_variable, expr, expr.identifier.lexeme);
        }
        return *val;
    }
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


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const CallExpr& expr) const {

    Value callee = evaluate(*expr.callee);

    std::vector<Value> args;
    args.reserve(expr.args.size());
    for (const auto& arg : expr.args) {
        args.emplace_back(evaluate(*arg));
    }

    if (holds<Function>(callee)) {
        return get_invokable<Function>(callee, args, expr)(*this, args);
    } else if (holds<BuiltinFunction>(callee)) {
        return get_invokable<BuiltinFunction>(callee, args, expr)(args);
    } else {
        report_error_and_abort(
            InterpreterError::unexpected_type, expr,
            fmt::format(
                "Expected {} or {}, Encountered {}",
                type_name(Value(Function{ nullptr })),
                type_name(Value(BuiltinFunction{ "", nullptr, 0 })),
                type_name(callee)
            )
        );
    }

    return { nullptr };
}



template<typename CallableValue>
CallableValue& ExprInterpreterVisitor::get_invokable(Value& callee, std::vector<Value>& args, const CallExpr& expr) const {
    CallableValue& function = std::get<CallableValue>(callee);

    if (function.arity() != args.size()) {
        report_error_and_abort(
            InterpreterError::wrong_num_of_arguments, expr,
            fmt::format(
                "Expected {}, Encountered {}",
                function.arity(),
                args.size()
            )
        );
    }

    return function;
}
