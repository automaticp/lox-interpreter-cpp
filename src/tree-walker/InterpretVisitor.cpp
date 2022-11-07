#include "InterpretVisitor.hpp"

#include "Environment.hpp"
#include "Interpreter.hpp"
#include "InterpreterError.hpp"
#include "Value.hpp"
#include <fmt/format.h>
#include <vector>









// Specialization of Function call with the Interpreter





template<>
Value Function::operator()<Interpreter>(Interpreter& interpreter, std::span<Value> args) {
    assert(declaration());
    // Environment from enclosing scope,
    // captured by copy during construction of Function
    Environment env{ &closure() };

    for (size_t i{ 0 }; i < args.size(); ++i) {
        env.define(
            declaration()->parameters[i].lexeme(), std::move(args[i])
        );
    }

    try {
        interpreter.interpret(
            declaration()->body, env
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









// Private methods





// Interprets the expression and decays the result.
// Decay collapses ValueHandle into the wrapped type.
Value InterpretVisitor::evaluate(const Expr& expr) const {
    return decay(expr.accept(*this));
}

// Interprets the expression without decaying the result.
// Used when value could be mutated through a reference:
// in methods, closures, assignment, etc.
Value InterpretVisitor::evaluate_without_decay(const Expr& expr) const {
    return expr.accept(*this);
}


void InterpretVisitor::execute(const Stmt& stmt) const {
    stmt.accept(*this);
}




bool InterpretVisitor::is_truthful(const Value& value) {
    if (value.is<Nil>()) {
        return false;
    }

    if (value.is<Boolean>()) {
        return value.as<Boolean>();
    } else {
        return true;
    }
}



template<typename T>
void InterpretVisitor::check_type(const Expr& expr, const Value& val) const {
    if (!val.is<T>()) {
        report_error_and_abort(
            InterpreterError::Type::unexpected_type, expr,
            fmt::format("Expected {:s}, Encountered {:s}", type_name(Value{T{}}), type_name(val))
        );
    }
}


template<typename T1, typename T2>
void InterpretVisitor::check_type(const Expr& expr, const Value& val1, const Value& val2) const {
    check_type<T1>(expr, val1);
    check_type<T2>(expr, val2);
}




void InterpretVisitor::report_error(InterpreterError::Type type, const Expr& expr, std::string_view details) const {
    interpreter_.send_error(type, expr, std::string(details));
}


void InterpretVisitor::report_error_and_abort(InterpreterError::Type type, const Expr& expr, std::string_view details) const {
    report_error(type, expr, details);
    interpreter_.abort_by_exception(type);
}


template<typename CallableValue>
CallableValue& InterpretVisitor::get_invokable(Value& callee, std::vector<Value>& args, const CallExpr& expr) const {
    CallableValue& function = callee.as<CallableValue>();

    if (function.arity() != args.size()) {
        report_error_and_abort(
            InterpreterError::Type::wrong_num_of_arguments, expr,
            fmt::format(
                "Expected {}, Encountered {}",
                function.arity(),
                args.size()
            )
        );
    }

    return function;
}







// Expr visitor overloads






Value InterpretVisitor::operator()(const LiteralExpr& expr) const {
    assert(expr.token.has_literal());
    return {
        std::visit(
            [](auto&& arg) { return Value{ arg }; },
            expr.token.literal()
        )
    };
}




Value InterpretVisitor::operator()(const UnaryExpr& expr) const {
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




Value InterpretVisitor::operator()(const BinaryExpr& expr) const {
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
                report_error_and_abort(
                    InterpreterError::Type::unexpected_type, expr,
                    fmt::format(
                        "Expected a pair of Numbers or Strings, Encountered {:s} and {:s}",
                        type_name(lhs), type_name(rhs)
                    )
                );
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




Value InterpretVisitor::operator()(const GroupedExpr& expr) const {
    // Does not decay, so that:
    //
    // fun f() { ... }
    //
    // (f)();
    //
    // still evaluates f as reference.
    return evaluate_without_decay(*expr.expr);
}




Value InterpretVisitor::operator()(const VariableExpr& expr) const {

    // FIXME now that the global scope is proper scope
    auto& depths = interpreter_.resolver_.depth_map();
    auto it = depths.find(&Expr::from_alternative(expr));

    ValueHandle handle{};

    if (it != depths.end()) {
        handle = env_.get_at(it->second, expr.identifier.lexeme());
    } else {
        handle = interpreter_.env_.get(expr.identifier.lexeme());
        if (!handle) {
            report_error_and_abort(
                InterpreterError::Type::undefined_variable,
                expr, expr.identifier.lexeme()
            );
        }
    }
    assert(handle);
    // Return ValueHandle directly
    return handle;
}




Value InterpretVisitor::operator()(const AssignExpr& expr) const {
    auto& depths = interpreter_.resolver_.depth_map();

    auto it = depths.find(&Expr::from_alternative(expr));
    if (it != depths.end()) {
        ValueHandle val = env_.assign_at(
            it->second,
            expr.identifier.lexeme(),
            evaluate(*expr.rvalue)
        );
        assert(val); // This whole thing is so fragile, I hate it
        return *val;
    } else {
        ValueHandle val = interpreter_.env_.assign(
            expr.identifier.lexeme(),
            evaluate(*expr.rvalue)
        );

        if (!val) {
            report_error_and_abort(
                InterpreterError::Type::undefined_variable, expr, expr.identifier.lexeme()
            );
        }
        return *val;
    }
}




Value InterpretVisitor::operator()(const LogicalExpr& expr) const {
    Value lhs{ evaluate(*expr.lhs) };

    if (expr.op.type() == TokenType::kw_or) {
        if (is_truthful(lhs)) { return lhs; }
    } else {
        if (!is_truthful(lhs)) { return lhs; }
    }

    return evaluate(*expr.rhs);
}




Value InterpretVisitor::operator()(const CallExpr& expr) const {

    Value callee_maybe_handle = evaluate_without_decay(*expr.callee);
    Value& callee = decay(callee_maybe_handle);

    std::vector<Value> args;
    args.reserve(expr.args.size());
    for (const auto& arg : expr.args) {
        args.emplace_back(evaluate(*arg));
    }

    if (callee.is<Function>()) {
        return get_invokable<Function>(callee, args, expr)(this->interpreter_, args);
    } else if (callee.is<BuiltinFunction>()) {
        return get_invokable<BuiltinFunction>(callee, args, expr)(this->interpreter_, args);
    } else {
        report_error_and_abort(
            InterpreterError::Type::unexpected_type, expr,
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








// Stmt visitor overloads







void InterpretVisitor::operator()(const PrintStmt& stmt) const {
    auto value = evaluate(*stmt.expr);
    std::cout << to_string(value) << '\n';
}




void InterpretVisitor::operator()(const ExpressionStmt& stmt) const {
    evaluate(*stmt.expr);
}




void InterpretVisitor::operator()(const VarStmt& stmt) const {
    env_.define(stmt.identifier.lexeme(), evaluate(*stmt.init));
}




void InterpretVisitor::operator()(const BlockStmt& stmt) const {
    Environment block_env{ &env_ };
    InterpretVisitor block_visitor{ interpreter_, block_env };

    for (const auto& statement : stmt.statements) {
        block_visitor.execute(*statement);
    }
}




void InterpretVisitor::operator()(const IfStmt& stmt) const {
    if (is_truthful(evaluate(*stmt.condition))) {
        execute(*stmt.then_branch);
    } else if (stmt.else_branch) {
        execute(*stmt.else_branch);
    }
}




void InterpretVisitor::operator()(const WhileStmt& stmt) const {
    while (is_truthful(evaluate(*stmt.condition))) {
        execute(*stmt.statement);
    }
}




static void flatten_into_closure(Environment& closure, const Environment* starting) {
    const Environment* enclosing{ starting };
    while (enclosing) {
        for (const auto& elem : enclosing->map()) {
            if (!closure.get(elem.first)) {
                closure.define(elem.first, elem.second);
            }
        }
        enclosing = enclosing->enclosing();
    }
}


void InterpretVisitor::operator()(const FunStmt& stmt) const {
    // Hail Mary closure that copies EVERYTHING from outer scopes,
    // essentially, storing the state of the entire program at capture time.
    // Absolutely horrible, but should work.

    // First, intialize without any enclosing scopes.
    Environment closure{ nullptr };

    // Ther recursively copy values for symbols not yet in closure,
    // starting from the current scope.
    flatten_into_closure(closure, &env_);

    // Add this function to the current environment.
    ValueHandle fun_handle = env_.define(
        stmt.name.lexeme(),
        Function{
            &stmt,
            std::move(closure)
        }
    );

    // Add a ValueHandle of the function to the closure.
    // The ValueHandle will properly decay
    // in the Environmet::get() method.
    fun_handle.unwrap_to<Function>().closure().define(
        stmt.name.lexeme(), fun_handle
    );
    // Beware: copying the Function into it's own closure
    // will leak memory. Do not copy/decay it here.

}




void InterpretVisitor::operator()(const ReturnStmt& stmt) const {
    // Walk up the call stack with exceptions.
    // To be caught in the Function::operator()
    throw evaluate(*stmt.expr);
}


