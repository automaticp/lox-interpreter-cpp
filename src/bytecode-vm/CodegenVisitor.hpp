#pragma once
#include "Chunk.hpp"
#include "ErrorReporter.hpp"
#include "IError.hpp"
#include "ErrorSender.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"






class CodegenVisitor : private ErrorSender<SimpleError> {
private:
    Chunk& chunk_;

public:
    CodegenVisitor(ErrorReporter& err, Chunk& chunk) :
        ErrorSender{ err }, chunk_{ chunk }
    {}

    void operator()(const LiteralExpr& expr) const;
    void operator()(const UnaryExpr& expr) const;
    void operator()(const BinaryExpr& expr) const;
    void operator()(const GroupedExpr& expr) const;
    void operator()(const VariableExpr& expr) const;
    void operator()(const AssignExpr& expr) const;
    void operator()(const LogicalExpr& expr) const;
    void operator()(const CallExpr& expr) const;

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
    Chunk& chunk() const noexcept { return chunk_; }
    void not_implemented(const Expr& expr) const;
    void not_implemented(const Stmt& stmt) const;

    void codegen(const Expr& expr) const {
        expr.accept(*this);
    }

    void codegen(const Stmt& stmt) const {
        stmt.accept(*this);
    }

};
