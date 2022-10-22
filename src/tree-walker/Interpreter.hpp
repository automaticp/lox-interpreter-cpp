#pragma once
#include "Environment.hpp"
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Expr.hpp"
#include "ExprVisitors.hpp"
#include "StmtVisitors.hpp"
#include "IExpr.hpp"
#include "Value.hpp"
#include "Stmt.hpp"
#include "Builtins.hpp"
#include <span>

class Interpreter {
private:
    ErrorReporter& err_;
    Environment env_;
    StmtInterpreterVisitor visitor_;

public:
    Interpreter(ErrorReporter& err) :
        err_{ err }, visitor_{ err, env_, *this }, env_{} {

        setup_builtins(env_);
    }

    bool interpret(std::span<const std::unique_ptr<IStmt>> statements) {
        try {
            for (const auto& statement : statements) {
                visitor_.execute(*statement);
            }
            return true;
        } catch (InterpreterError) {
            return false;
        }
    }

    bool interpret(std::span<const std::unique_ptr<IStmt>> statements, Environment& env) {
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

};
