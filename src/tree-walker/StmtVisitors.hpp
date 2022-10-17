#pragma once
#include "ErrorReporter.hpp"
#include "ExprVisitors.hpp"

class IStmt;
class PrintStmt;
class ExpressionStmt;


// Inheritance is a hack here
struct StmtInterpreterVisitor : protected ExprInterpreterVisitor {
private:
    ErrorReporter& err_;
public:
    StmtInterpreterVisitor(ErrorReporter& err) :
        err_{ err }, ExprInterpreterVisitor{ err }
    {}

    using return_type = void;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;

    void execute(const IStmt& stmt) const;
};




struct StmtASTPrinterVisitor : protected ExprASTPrinterVisitor {
public:
    using return_type = std::string;

    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
};

