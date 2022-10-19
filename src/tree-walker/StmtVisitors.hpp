#pragma once
#include "Environment.hpp"
#include "ErrorReporter.hpp"
#include "ExprVisitors.hpp"

class IStmt;
class PrintStmt;
class ExpressionStmt;
class VarStmt;
class BlockStmt;

// Inheritance is a hack here
struct StmtInterpreterVisitor : protected ExprInterpreterVisitor {
private:
    ErrorReporter& err_;

public:
    StmtInterpreterVisitor(ErrorReporter& err, Environment& env) :
        err_{ err }, ExprInterpreterVisitor{ err, env }
    {}

    using return_type = void;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
    return_type operator()(const VarStmt& stmt) const;
    return_type operator()(const BlockStmt& stmt) const;

    void execute(const IStmt& stmt) const;
};




struct StmtASTPrinterVisitor : protected ExprASTPrinterVisitor {
public:
    using return_type = std::string;

    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
    return_type operator()(const VarStmt& stmt) const;
    return_type operator()(const BlockStmt& stmt) const;
};

