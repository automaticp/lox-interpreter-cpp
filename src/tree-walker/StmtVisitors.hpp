#pragma once
#include "ErrorReporter.hpp"

class IStmt;
class PrintStmt;
class ExpressionStmt;


struct StmtEvaluateVisitor {
private:
    ErrorReporter& err_;
public:
    StmtEvaluateVisitor(ErrorReporter& err) : err_{ err } {}

    using return_type = void;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
};
