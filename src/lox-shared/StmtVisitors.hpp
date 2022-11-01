#pragma once
#include "ExprVisitors.hpp"
#include "StmtInterpreterVisitor.hpp"
#include "StmtResolveVisitor.hpp"

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



struct StmtASTPrinterVisitor : public ExprASTPrinterVisitor {
public:
    using ExprASTPrinterVisitor::operator();
    using return_type = std::string;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
    return_type operator()(const VarStmt& stmt) const;
    return_type operator()(const BlockStmt& stmt) const;
    return_type operator()(const IfStmt& stmt) const;
    return_type operator()(const WhileStmt& stmt) const;
    return_type operator()(const FunStmt& stmt) const;
    return_type operator()(const ReturnStmt& stmt) const;
};


struct StmtUserFriendlyNameVisitor {
public:
    using return_type = std::string_view;
    return_type operator()(const PrintStmt& stmt) const;
    return_type operator()(const ExpressionStmt& stmt) const;
    return_type operator()(const VarStmt& stmt) const;
    return_type operator()(const BlockStmt& stmt) const;
    return_type operator()(const IfStmt& stmt) const;
    return_type operator()(const WhileStmt& stmt) const;
    return_type operator()(const FunStmt& stmt) const;
    return_type operator()(const ReturnStmt& stmt) const;
};

