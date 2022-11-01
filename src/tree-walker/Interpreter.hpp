#pragma once
#include "Environment.hpp"
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Expr.hpp"
#include "ExprVisitors.hpp"
#include "StmtVisitors.hpp"
#include "ExprInterpreterVisitor.hpp"
#include "StmtInterpreterVisitor.hpp"
#include "Value.hpp"
#include "Stmt.hpp"
#include "Builtins.hpp"
#include "Resolver.hpp"
#include <span>

class Interpreter {
private:
    ErrorReporter& err_;
    Resolver& resolver_;
    Environment env_;
    StmtInterpreterVisitor visitor_;

    // Another hack that shows the fragility of the design
    friend StmtInterpreterVisitor;
    friend ExprInterpreterVisitor;

public:
    Interpreter(ErrorReporter& err, Resolver& resolver) :
        err_{ err }, resolver_{ resolver }, visitor_{ err, env_, *this }, env_{}
    {}

    bool interpret(std::span<const std::unique_ptr<Stmt>> statements) {
        try {
            for (const auto& statement : statements) {
                visitor_.execute(*statement);
            }
            return true;
        } catch (InterpreterError) {
            return false;
        }
    }

    bool interpret(std::span<const std::unique_ptr<Stmt>> statements, Environment& env) {
        try {
            StmtInterpreterVisitor local_visitor{ err_, env, *this };
            for (const auto& statement : statements) {
                local_visitor.execute(*statement);
            }
            return true;
        } catch (InterpreterError) {
            return false;
        }
    }

    Environment& get_global_environment() noexcept { return env_; }

};
