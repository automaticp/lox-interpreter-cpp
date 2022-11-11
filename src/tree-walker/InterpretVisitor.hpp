#pragma once
#include "Expr.hpp"
#include "Stmt.hpp"
#include "InterpreterError.hpp"


class Interpreter;
class Environment;
class Value;


class InterpretVisitor {
private:
    Interpreter& interpreter_;
    Environment& env_;

public:
    InterpretVisitor(Interpreter& interpreter, Environment& env) :
        interpreter_{ interpreter }, env_{ env } {}

    // Expr visitor overloads

    Value operator()(const LiteralExpr& expr) const;
    Value operator()(const UnaryExpr& expr) const;
    Value operator()(const BinaryExpr& expr) const;
    Value operator()(const GroupedExpr& expr) const;
    Value operator()(const VariableExpr& expr) const;
    Value operator()(const AssignExpr& expr) const;
    Value operator()(const LogicalExpr& expr) const;
    Value operator()(const CallExpr& expr) const;

    // Stmt visitor overloads
    // Note: the return types are different.

    void operator()(const PrintStmt& stmt) const;
    void operator()(const ExpressionStmt& stmt) const;
    void operator()(const VarStmt& stmt) const;
    void operator()(const BlockStmt& stmt) const;
    void operator()(const IfStmt& stmt) const;
    void operator()(const WhileStmt& stmt) const;
    void operator()(const FunStmt& stmt) const;
    void operator()(const ReturnStmt& stmt) const;
    void operator()(const ImportStmt& stmt) const;

private:
    Value evaluate(const Expr& expr) const;
    Value evaluate_without_decay(const Expr& expr) const;

    void execute(const Stmt& stmt) const;


    static bool is_truthful(const Value& value);


    template<typename T>
    void check_type(const Expr& expr, const Value& val) const;

    template<typename T1, typename T2>
    void check_type(const Expr& expr, const Value& val1, const Value& val2) const;

    void report_error(InterpreterError::Type type, const Expr& expr, std::string_view details = "") const;

    void report_error_and_abort(InterpreterError::Type type, const Expr& expr, std::string_view details = "") const;

    template<typename CallableValue>
    CallableValue& get_invokable(Value& callee, std::vector<Value>& args, const CallExpr& expr) const;


};


