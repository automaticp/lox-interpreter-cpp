#include "IRGenVisitor.hpp"
#include "Expr.hpp"
#include "IRGen.hpp"
#include "TokenType.hpp"
#include <fmt/format.h>
#include <llvm-14/llvm/ADT/StringExtras.h>
#include <llvm-14/llvm/Support/Casting.h>
#include <llvm-14/llvm/Support/FormattedStream.h>
#include <llvm-14/llvm/Support/raw_ostream.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/IR/Value.h>

llvm::Value* IRGenVisitor::operator()(const LiteralExpr& expr) const {
    return llvm::ConstantFP::get(
        gen_.context_,
        llvm::APFloat{
            std::get<Number>(expr.token.literal())
        }
    );
}


llvm::Value* IRGenVisitor::operator()(const VariableExpr& expr) const {
    auto value = gen_.env_[expr.identifier.lexeme()];

    if (!value) {
        gen_.send_error(
            fmt::format(
                "[Error @IRGen]: Unknown var: {}",
                expr.identifier.lexeme()
            )
        );
    }

    return value;
}


llvm::Value* IRGenVisitor::operator()(const BinaryExpr& expr) const {
    auto lhs = expr.lhs->accept(*this);
    auto rhs = expr.rhs->accept(*this);

    if (!lhs || !rhs) return {};

    switch (expr.op.type()) {
        case TokenType::plus:
            return gen_.builder_.CreateFAdd(lhs, rhs, "fadd");
        case TokenType::minus:
            return gen_.builder_.CreateFSub(lhs, rhs, "fsub");
        case TokenType::star:
            return gen_.builder_.CreateFMul(lhs, rhs, "fmul");
        case TokenType::slash:
            return gen_.builder_.CreateFDiv(lhs, rhs, "fdiv");
        default:
            gen_.send_error(
                fmt::format(
                    "Invalid binary op: {}",
                    expr.op.lexeme()
                )
            );
            return {};
    }
}


void IRGenVisitor::operator()(const PrintStmt& expr) const {
    auto arg = expr.expr->accept(*this);
    // Ehh, well, this is codegen, why do I expect a value?
    std::cout << arg->getName().str() << '\n';
}




llvm::Value* IRGenVisitor::operator()(const UnaryExpr& expr) const { return {}; }
llvm::Value* IRGenVisitor::operator()(const GroupedExpr& expr) const { return {}; }
llvm::Value* IRGenVisitor::operator()(const AssignExpr& expr) const { return {}; }
llvm::Value* IRGenVisitor::operator()(const LogicalExpr& expr) const { return {}; }
llvm::Value* IRGenVisitor::operator()(const CallExpr& expr) const { return {}; }

void IRGenVisitor::operator()(const ExpressionStmt& stmt) const {}
void IRGenVisitor::operator()(const VarStmt& stmt) const {}
void IRGenVisitor::operator()(const BlockStmt& stmt) const {}
void IRGenVisitor::operator()(const IfStmt& stmt) const {}
void IRGenVisitor::operator()(const WhileStmt& stmt) const {}
void IRGenVisitor::operator()(const FunStmt& stmt) const {}
void IRGenVisitor::operator()(const ReturnStmt& stmt) const {}
