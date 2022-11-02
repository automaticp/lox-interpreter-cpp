#pragma once
#include "Environment.hpp"
#include "ErrorReporter.hpp"
#include "InterpreterError.hpp"
#include "ErrorSender.hpp"
#include "Expr.hpp"
#include "CommonVisitors.hpp"
#include "ExprInterpreterVisitor.hpp"
#include "StmtInterpreterVisitor.hpp"
#include "Value.hpp"
#include "Stmt.hpp"
#include "Builtins.hpp"
#include "Resolver.hpp"
#include <span>

class Interpreter : private ErrorSender<InterpreterError> {
private:
    Resolver& resolver_;
    Environment env_;
    StmtInterpreterVisitor visitor_;

    // Another hack that shows the fragility of the design
    friend StmtInterpreterVisitor;
    friend ExprInterpreterVisitor;

public:
    Interpreter(ErrorReporter& err, Resolver& resolver) :
        ErrorSender{ err }, resolver_{ resolver }, visitor_{ env_, *this }, env_{}
    {}

    bool interpret(std::span<const std::unique_ptr<Stmt>> statements) {
        try {
            for (const auto& statement : statements) {
                visitor_.execute(*statement);
            }
            return true;
        } catch (InterpreterError::Type) {
            return false;
        }
    }

    bool interpret(std::span<const std::unique_ptr<Stmt>> statements, Environment& env) {
        try {
            StmtInterpreterVisitor local_visitor{ env, *this };
            for (const auto& statement : statements) {
                local_visitor.execute(*statement);
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
