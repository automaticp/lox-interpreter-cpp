#pragma once
#include "ExprInterpreterVisitor.hpp"

class Stmt;
class PrintStmt;
class ExpressionStmt;
class VarStmt;
class BlockStmt;
class IfStmt;
class WhileStmt;
class ReturnStmt;

class Environment;
class ErrorReporter;
class Interpreter;


// Inheritance is a hack here
struct StmtInterpreterVisitor : protected ExprInterpreterVisitor {
//
public:
    StmtInterpreterVisitor(ErrorReporter& err, Environment& env, Interpreter& interpreter) :
        ExprInterpreterVisitor{ err, env, interpreter }
    {}

    using return_type = void;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
    return_type operator()(const VarStmt& stmt) const;
    return_type operator()(const BlockStmt& stmt) const;
    return_type operator()(const IfStmt& stmt) const;
    return_type operator()(const WhileStmt& stmt) const;
    return_type operator()(const FunStmt& stmt) const;
    return_type operator()(const ReturnStmt& stmt) const;

    void execute(const Stmt& stmt) const;
};

