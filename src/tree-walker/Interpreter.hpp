#pragma once
#include "InterpretVisitor.hpp"
#include "InterpreterError.hpp"
#include "ErrorSender.hpp"
#include "Environment.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Value.hpp"
#include "Resolver.hpp"
#include <span>
#include <memory>



class Interpreter : private ErrorSender<InterpreterError> {
private:
    Resolver& resolver_;
    Environment env_;

    friend InterpretVisitor;
    InterpretVisitor visitor_;

public:
    Interpreter(ErrorReporter& err, Resolver& resolver) :
        ErrorSender{ err },
        resolver_{ resolver },
        env_{},
        visitor_{ *this, env_ }
    {}

    bool interpret(std::span<const std::unique_ptr<Stmt>> statements) {
        try {
            for (const auto& statement : statements) {
                statement->accept(visitor_);
            }
            return true;
        } catch (InterpreterError::Type) {
            return false;
        }
    }

    bool interpret(std::span<const std::unique_ptr<Stmt>> statements, Environment& env) {
        try {
            InterpretVisitor local_visitor{ *this, env };
            for (const auto& statement : statements) {
                statement->accept(local_visitor);
            }
            return true;
        } catch (InterpreterError::Type) {
            return false;
        }
    }

    Environment& get_global_environment() noexcept { return env_; }

private:
    void abort_by_exception(InterpreterError::Type type) const noexcept(false) {
        throw type;
    }
};




















// Am I the only one who finds the word 'Interpreter' really hard to spell?











































// Interrprpperpepreprter
