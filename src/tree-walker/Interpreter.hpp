#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Expr.hpp"
#include "ExprVisitors.hpp"
#include "StmtVisitors.hpp"
#include "IExpr.hpp"
#include "Value.hpp"
#include "Stmt.hpp"
#include <vector>

class Interpreter {
private:
    ErrorReporter& err_;
    StmtInterpreterVisitor visitor_;

public:
    Interpreter(ErrorReporter& err) :
        err_{ err }, visitor_{ err }
    {}

    bool interpret(const std::vector<std::unique_ptr<IStmt>>& statements) {
        try {
            for (const auto& statement : statements) {
                visitor_.execute(*statement);
            }
            return true;
        } catch (InterpreterError) {
            return false;
        }
    }

};
