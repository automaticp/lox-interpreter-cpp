#pragma once
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"


class ASTPrintVisitor {
public:

    // Expr visitor overloads

    std::string operator()(const LiteralExpr& expr) const {
        assert(expr.token.has_literal());
        return to_string(expr.token.literal());
    }

    std::string operator()(const UnaryExpr& expr) const {
        return parenthesize(to_lexeme(expr.op), *expr.operand);
    }

    std::string operator()(const BinaryExpr& expr) const {
        return parenthesize(to_lexeme(expr.op), *expr.lhs, *expr.rhs);
    }

    std::string operator()(const GroupedExpr& expr) const {
        return parenthesize("group", *expr.expr);
    }

    std::string operator()(const VariableExpr& expr) const {
        return expr.identifier.lexeme();
    }

    std::string operator()(const AssignExpr& expr) const {
        return parenthesize(expr.identifier.lexeme(), *expr.rvalue);
    }

    std::string operator()(const LogicalExpr& expr) const {
        return parenthesize(to_lexeme(expr.op), *expr.lhs, *expr.rhs);
    }

    std::string operator()(const CallExpr& expr) const {
        return call_expr_string(expr);
    }




    // Stmt visitor overloads

    std::string operator()(const PrintStmt& stmt) const {
        return fmt::format("print {:s};", stmt.expr->accept(*this));
    }

    std::string operator()(const ExpressionStmt& stmt) const {
        return fmt::format("{:s};", stmt.expr->accept(*this));
    }

    std::string operator()(const VarStmt& stmt) const {
        return fmt::format("var {:s} = {:s};", stmt.identifier.lexeme(), stmt.init->accept(*this));
    }

    std::string operator()(const BlockStmt& stmt) const {
        std::string result{ "{\n" };
        for (const auto& statement : stmt.statements) {
            result += statement->accept(*this) + '\n';
        }
        result += "}";
        return result;
    }

    std::string operator()(const IfStmt& stmt) const {
        return fmt::format(
            "if ({}) {} {} {}",
            stmt.condition->accept(*this),
            stmt.then_branch->accept(*this),
            stmt.else_branch ? "else" : "",
            stmt.else_branch ? stmt.else_branch->accept(*this) : ""
        );
    }

    std::string operator()(const WhileStmt& stmt) const {
        return fmt::format(
            "while ({}) {}",
            stmt.condition->accept(*this),
            stmt.statement->accept(*this)
        );
    }

    std::string operator()(const FunStmt& stmt) const {
        // I wanted to do ranges but it seems I don't know ranges =(
        auto join_token_names = [](const std::vector<Token>& ts) -> std::string {
            if (ts.empty()) return { "" };

            std::string result;
            auto it{ ts.begin() };

            for (; it < ts.end() - 1; ++it) {
                result += it->lexeme() + ", ";
            }
            result += (++it)->lexeme();

            return result;
        };

        auto join_statements = [this](const std::vector<std::unique_ptr<Stmt>>& stmts) -> std::string {
            std::string result;
            for (const auto& s : stmts) {
                result += s->accept(*this) + '\n';
            }
            return result;
        };

        return fmt::format(
            "fun {}({}) {{\n{}}}",
            stmt.name.lexeme(),
            join_token_names(stmt.parameters),
            join_statements(stmt.body)
        );
    }

    std::string operator()(const ReturnStmt& stmt) const {
        return fmt::format(
            "return {};", stmt.expr->accept(*this)
        );
    }


    std::string operator()(const ImportStmt& stmt) const {
        return fmt::format(
            "import {};", stmt.path.lexeme()
        );
    }


private:
    template<std::derived_from<Expr> ...Es>
    std::string parenthesize(std::string_view name, const Es&... exprs) const {
        std::stringstream ss;
        ss << '(' << name;
        (ss << ... << (std::string(" ") + exprs.accept(*this)));
        ss << ')';
        return ss.str();
    }

    std::string call_expr_string(const CallExpr& expr) const {
        std::string result{ "(call " };
        result += expr.callee->accept(*this) + " ";
        for (const auto& arg : expr.args) {
            result += arg->accept(*this) + " ";
        }
        result += ")";
        return result;
    }

};










class UserFriendlyNameVisitor {
public:
    using result_t = std::string_view;


    // Expr visitor overloads

    result_t operator()(const LiteralExpr&) const {
        return "Literal Expression";
    }

    result_t operator()(const UnaryExpr&) const {
        return "Unary Expression";
    }

    result_t operator()(const BinaryExpr&) const {
        return "Binary Expression";
    }

    result_t operator()(const GroupedExpr&) const {
        return "Group Expression";
    }

    result_t operator()(const VariableExpr&) const {
        return "Variable Expression";
    }

    result_t operator()(const AssignExpr&) const {
        return "Assignment Expression";
    }

    result_t operator()(const LogicalExpr&) const {
        return "Logical Expression";
    }

    result_t operator()(const CallExpr&) const {
        return "Call Expression";
    }


    // Stmt visitor overloads

    result_t operator()(const PrintStmt&) const {
        return "Print Statement";
    }

    result_t operator()(const ExpressionStmt&) const {
        return "Expression Statement";
    }

    result_t operator()(const VarStmt&) const {
        return "Variable Declaration";
    }

    result_t operator()(const BlockStmt&) const {
        return "Block Statement";
    }

    result_t operator()(const IfStmt&) const {
        return "If Statement";
    }

    result_t operator()(const WhileStmt&) const {
        return "Loop Statement";
    }

    result_t operator()(const FunStmt&) const {
        return "Function Declaration";
    }

    result_t operator()(const ReturnStmt&) const {
        return "Return Statement";
    }

    result_t operator()(const ImportStmt&) const {
        return "Import Statement";
    }

};


inline std::string name_of(const auto& expr_or_stmt) {
    // Convert to string from string_view as well
    return std::string(expr_or_stmt.accept(UserFriendlyNameVisitor{}));
}









// Only supports Expr
class ExprGetPrimaryTokenVisitor {
public:
    using result_t = const Token&;

    result_t operator()(const LiteralExpr& expr) const {
        return expr.token;
    }

    result_t operator()(const UnaryExpr& expr) const {
        return expr.op;
    }

    result_t operator()(const BinaryExpr& expr) const {
        return expr.op;
    }

    result_t operator()(const GroupedExpr& expr) const {
        return expr.expr->accept(*this);
    }

    result_t operator()(const VariableExpr& expr) const {
        return expr.identifier;
    }

    result_t operator()(const AssignExpr& expr) const {
        return expr.op;
    }

    result_t operator()(const LogicalExpr& expr) const {
        return expr.op;
    }

    result_t operator()(const CallExpr& expr) const {
        return expr.rparen;
    }

};



inline decltype(auto) primary_token_of(const Expr& expr) {
    return expr.accept(ExprGetPrimaryTokenVisitor{});
}


