#include "ExprInterpreterVisitor.hpp"

#include "Expr.hpp"
#include "TokenType.hpp"
#include "Value.hpp"
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Environment.hpp"
#include "Interpreter.hpp"
#include "ValueDecl.hpp"
#include <fmt/format.h>
#include <variant>


// Specialization of Function call with the Interpreter
template<>
Value Function::operator()<Interpreter>(Interpreter& interpreter, std::span<Value> args) {
    assert(declaration_);
    // Environment from enclosing scope,
    // captured by copy during construction of Function
    Environment env{ &closure_ };

    for (size_t i{ 0 }; i < args.size(); ++i) {
        env.define(
            declaration_->parameters[i].lexeme, std::move(args[i])
        );
    }

    try {
        interpreter.interpret(
            declaration_->body, env
        );
    } catch (Value& v) {
        return std::move(v);
    }

    return {};
}


template<>
Value BuiltinFunction::operator()<Interpreter>(Interpreter&, std::span<Value> args) {
    return fun_(args);
}





// Interprets the expression and decays the result.
// Decay collapses ValueHandle into the wrapped type.
ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::evaluate(const Expr& expr) const {
    return decay(expr.accept(*this));
}

// Interprets the expression without decaying the result.
// Used when value could be mutated through a reference:
// in methods, closures, assignment, etc.
ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::evaluate_without_decay(const Expr& expr) const {
    return expr.accept(*this);
}


bool ExprInterpreterVisitor::is_truthful(const Value& value) {
    if (value.is<Nil>()) {
        return false;
    }

    if (value.is<Boolean>()) {
        return value.as<Boolean>();
    } else {
        return true;
    }
}


void ExprInterpreterVisitor::report_error(InterpreterError type, const Expr& expr, std::string_view details) const {
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
            check_type<Number>(expr, val);
            return -val.as<Number>();
            break;
        case TokenType::plus:
            check_type<Number>(expr, val);
            break;
        case TokenType::bang:
            return !is_truthful(val);
            break;
        default:
            // FIXME: report failure?
            break;
    }
    return {};
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const BinaryExpr& expr) const {
    Value lhs{ evaluate(*expr.lhs) };
    Value rhs{ evaluate(*expr.rhs) };

    using enum TokenType;
    switch (expr.op) {
        case minus:
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() - rhs.as<Number>();
        case slash:
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() / rhs.as<Number>();
        case star:
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() * rhs.as<Number>();
        case plus:
            if (lhs.is<Number>() && rhs.is<Number>()) {
                return lhs.as<Number>() + rhs.as<Number>();
            } else if (lhs.is<String>() && rhs.is<String>()) {
                return lhs.as<String>() + rhs.as<String>();
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
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() > rhs.as<Number>();
        case greater_eq:
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() >= rhs.as<Number>();
        case less:
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() < rhs.as<Number>();
        case less_eq:
            check_type<Number, Number>(expr, lhs, rhs);
            return lhs.as<Number>() <= rhs.as<Number>();
        case eq_eq:
            return lhs == rhs;
        case bang_eq:
            return lhs != rhs;
        default:
            break;
    }

    return {};
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const GroupedExpr& expr) const {
    // Does not decay, so that:
    //
    // fun f() { ... }
    //
    // (f)();
    //
    // still evaluates f as reference.
    return evaluate_without_decay(*expr.expr);
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const VariableExpr& expr) const {

    // FIXME now that the global scope is proper scope
    auto& depths = interpreter.resolver_.depth_map();
    auto it = depths.find(&Expr::from_alternative(expr));

    ValueHandle handle{};

    if (it != depths.end()) {
        handle = env.get_at(it->second, expr.identifier.lexeme);
    } else {
        handle = interpreter.env_.get(expr.identifier.lexeme);
        if (!handle) {
            report_error_and_abort(
                InterpreterError::undefined_variable,
                expr, expr.identifier.lexeme
            );
        }
    }
    assert(handle);
    // Return ValueHandle directly
    return handle;
}


ExprInterpreterVisitor::return_type
ExprInterpreterVisitor::operator()(const AssignExpr& expr) const {
    auto& depths = interpreter.resolver_.depth_map();

    auto it = depths.find(&Expr::from_alternative(expr));
    if (it != depths.end()) {
        ValueHandle val = env.assign_at(
            it->second,
            expr.identifier.lexeme,
            evaluate(*expr.rvalue)
        );
        assert(val); // This whole thing is so fragile, I hate it
        return *val;
    } else {
        ValueHandle val = interpreter.env_.assign(
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

    Value callee_maybe_handle = evaluate_without_decay(*expr.callee);
    Value& callee = decay(callee_maybe_handle);

    std::vector<Value> args;
    args.reserve(expr.args.size());
    for (const auto& arg : expr.args) {
        args.emplace_back(evaluate(*arg));
    }

    if (callee.is<Function>()) {
        return get_invokable<Function>(callee, args, expr)(this->interpreter, args);
    } else if (callee.is<BuiltinFunction>()) {
        return get_invokable<BuiltinFunction>(callee, args, expr)(this->interpreter, args);
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

    return {};
}



template<typename CallableValue>
CallableValue& ExprInterpreterVisitor::get_invokable(Value& callee, std::vector<Value>& args, const CallExpr& expr) const {
    CallableValue& function = callee.as<CallableValue>();

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
