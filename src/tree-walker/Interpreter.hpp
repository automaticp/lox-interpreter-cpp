#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Expr.hpp"
#include "ExprVisitors.hpp"
#include "IExpr.hpp"
#include "Value.hpp"

class Interpreter {
private:
    const IExpr& root_expr_;
    ErrorReporter& err_;
    Value result_{ nullptr };

public:
    friend class ExprInterpreterVisitor;

    Interpreter(const IExpr& root_expr, ErrorReporter& err) :
        root_expr_{ root_expr }, err_{ err }
    {}

    bool interpret() {
        try {
            result_ = root_expr_.accept(ExprInterpreterVisitor{ err_ });
            return true;
        } catch (InterpreterError) {
            return false;
        }
    }

    const Value& result() const noexcept { return result_; }


};
