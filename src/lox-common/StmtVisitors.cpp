#include "StmtVisitors.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "ExprVisitors.hpp"
#include "Token.hpp"
#include "Value.hpp"
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <ranges>




StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const PrintStmt& stmt) const {
    return fmt::format("print {:s};", stmt.expr->accept(*this));
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const ExpressionStmt& stmt) const {
    return fmt::format("{:s};", stmt.expr->accept(*this));
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const VarStmt& stmt) const {
    return fmt::format("var {:s} = {:s};", stmt.identifier.lexeme, stmt.init->accept(*this));
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const BlockStmt& stmt) const {
    std::string result{ "{\n" };
    for (const auto& statement : stmt.statements) {
        result += statement->accept(*this) + '\n';
    }
    result += "}";
    return result;
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const IfStmt& stmt) const {
    return fmt::format(
        "if ({}) {} {} {}",
        stmt.condition->accept(*this),
        stmt.then_branch->accept(*this),
        stmt.else_branch ? "else" : "",
        stmt.else_branch ? stmt.else_branch->accept(*this) : ""
    );
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const WhileStmt& stmt) const {
    return fmt::format(
        "while ({}) {}",
        stmt.condition->accept(*this),
        stmt.statement->accept(*this)
    );
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const FunStmt& stmt) const {
    // I wanted to do ranges but it seems I don't know ranges =(
    auto join_token_names = [](const std::vector<Token>& ts) -> std::string {
        if (ts.empty()) return { "" };

        std::string result;
        auto it{ ts.begin() };

        for (; it < ts.end() - 1; ++it) {
            result += it->lexeme + ", ";
        }
        result += (++it)->lexeme;

        return result;
    };

    auto join_statements = [this](const std::vector<std::unique_ptr<Stmt>>& stmts) -> std::string {
        std::string result;
        for (const auto& s : stmts) {
            result += s->accept(*this);
        }
        return result;
    };

    return fmt::format(
        "fun {}({}) {{\n{}}}\n",
        stmt.name.lexeme,
        join_token_names(stmt.parameters),
        join_statements(stmt.body)
    );
}

StmtASTPrinterVisitor::return_type
StmtASTPrinterVisitor::operator()(const ReturnStmt& stmt) const {
    return fmt::format(
        "return {};", stmt.expr->accept(*this)
    );
}








StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const PrintStmt&) const {
    return "Print Statement";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const ExpressionStmt&) const {
    return "Expression Statement";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const VarStmt&) const {
    return "Variable Declaration";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const BlockStmt&) const {
    return "Block Statement";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const IfStmt&) const {
    return "If Statement";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const WhileStmt&) const {
    return "Loop Statement";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const FunStmt&) const {
    return "Function Declaration";
}

StmtUserFriendlyNameVisitor::return_type
StmtUserFriendlyNameVisitor::operator()(const ReturnStmt&) const {
    return "Return Statement";
}


std::string name_of(const Stmt& stmt) {
    return std::string(stmt.accept(StmtUserFriendlyNameVisitor{}));
}
