#pragma once
#include "Expr.hpp"
#include "Stmt.hpp"
#include <llvm/IR/Value.h>

class IRGen;


class IRGenVisitor {
private:
    IRGen& gen_;
public:
    IRGenVisitor(IRGen& gen) : gen_{ gen } {}

    llvm::Value* operator()(const LiteralExpr& expr) const;
    llvm::Value* operator()(const UnaryExpr& expr) const;
    llvm::Value* operator()(const BinaryExpr& expr) const;
    llvm::Value* operator()(const GroupedExpr& expr) const;
    llvm::Value* operator()(const VariableExpr& expr) const;
    llvm::Value* operator()(const AssignExpr& expr) const;
    llvm::Value* operator()(const LogicalExpr& expr) const;
    llvm::Value* operator()(const CallExpr& expr) const;

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



};
