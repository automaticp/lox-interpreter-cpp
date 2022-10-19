#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "ExprVisitors.hpp"
#include "IExpr.hpp"
#include "Expr.hpp"
#include "IStmt.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "TokenType.hpp"
#include <concepts>
#include <vector>
#include <cassert>
#include <concepts>
#include <memory>
#include <utility>


class Parser {
private:
    class ParserState {
    public:
        using iter_t = std::vector<Token>::const_iterator;
    private:
        const iter_t beg_;
        iter_t cur_;
        const iter_t end_;

    public:
        ParserState(iter_t beg, iter_t end) :
            beg_{ beg }, cur_{ beg }, end_{ end } {}

        void reset() noexcept { cur_ = beg_; }

        bool is_end() const noexcept { return cur_ == end_; }

        bool is_eof() const noexcept { return cur_->type == TokenType::eof; }

        bool is_begin() const noexcept { return cur_ == beg_; }

        bool is_next_end() const noexcept { return cur_ + 1 == end_; }

        iter_t begin() const noexcept { return beg_; }

        iter_t end() const noexcept { return end_; }

        iter_t current() const noexcept { return cur_; }

        iter_t previous() const noexcept { return cur_ - 1; }

        iter_t next() const noexcept { return cur_ + 1; }

        const Token& peek_previous() const noexcept {
            assert(!is_begin());
            return *previous();
        }

        const Token& peek() const noexcept {
            assert(!is_end());
            return *current();
        }

        const Token& peek_next() const noexcept {
            assert(!is_end());
            assert(!is_next_end());
            return *next();
        }

        const Token& advance() noexcept {
            assert(!is_end());
            return *cur_++;
        }

        bool check(TokenType type) const noexcept {
            return peek().type == type;
        }

        bool match(TokenType expected) noexcept {
            if (is_end()) return false;

            if (check(expected)) {
                advance();
                return true;
            } else {
                return false;
            }
        }

        template<std::same_as<TokenType> ...TokenTypes>
        bool match(TokenTypes... types) noexcept {
            if (is_end()) return false;

            return (... || match(types));
        }

    };



    std::vector<Token> tokens_;
    ErrorReporter& err_;
    std::vector<std::unique_ptr<IStmt>> statements_;

    ParserState state_{ tokens_.begin(), tokens_.end() };





public:
    Parser(std::vector<Token> tokens, ErrorReporter& err) :
        tokens_{ std::move(tokens) }, err_{ err } {}

    bool parse_tokens() {
        statements_.clear();

        while (!state_.is_eof()) {
            try {
                statements_.emplace_back(declaration());
            } catch (ParserError) {
                synchronize_on_next_statement();
            }
        }
        return !err_.had_parser_errors();
    }

    const std::vector<std::unique_ptr<IStmt>>& peek_result() const {
        assert(is_full());
        return statements_;
    }

    [[nodiscard]] std::vector<std::unique_ptr<IStmt>> get_result() {
        assert(is_full());
        state_.reset();
        return std::move(statements_);
    }

    bool is_full() const noexcept {
        return !tokens_.empty() && state_.is_eof();
    }




private:
    std::unique_ptr<IStmt> declaration() {
        if (state_.match(TokenType::kw_var)) {
            return var_decl();
        } else {
            return statement();
        }
    }

    std::unique_ptr<IStmt> var_decl() {
        const Token& id = try_consume(
            TokenType::identifier, ParserError::expected_identifier
        );

        // FIXME: could be LiteralExpr(nil) or its own NullExpr;
        std::unique_ptr<IExpr> init;
        if (state_.match(TokenType::eq)) {
            init = expression();
        } else {
            init = std::make_unique<LiteralExpr>(
                Token{TokenType::kw_nil, "nil", state_.current()->line, nullptr}
            );
        }

        try_consume_semicolon();
        return std::make_unique<VarStmt>(id, std::move(init));
    }

    std::unique_ptr<IStmt> statement() {
        if (state_.match(TokenType::kw_if)) {
            return if_stmt();
        } else if (state_.match(TokenType::kw_while)) {
            return while_stmt();
        } else if (state_.match(TokenType::kw_for)) {
            return for_stmt();
        } else if (state_.match(TokenType::kw_print)) {
            return print_stmt();
        } else if (state_.match(TokenType::lbrace)) {
            return block_stmt();
        } else {
            return expression_stmt();
        }
    }

    std::unique_ptr<IStmt> if_stmt() {
        try_consume(
            TokenType::lparen, ParserError::missing_opening_paren
        );

        auto condition = expression();

        try_consume(
            TokenType::rparen, ParserError::missing_closing_paren
        );

        auto then_branch = statement();
        std::unique_ptr<IStmt> else_branch{ nullptr };
        if (state_.match(TokenType::kw_else)) {
            else_branch = statement();
        }

        return std::make_unique<IfStmt>(
            std::move(condition),
            std::move(then_branch),
            std::move(else_branch)
        );
    }

    std::unique_ptr<IStmt> for_stmt() {
        try_consume(
            TokenType::lparen, ParserError::missing_opening_paren
        );


        std::unique_ptr<IStmt> initializer{ nullptr };
        if (state_.match(TokenType::semicolon)) {
            /* nothing, already null */
        } else if (state_.match(TokenType::kw_var)) {
            initializer = var_decl();
        } else {
            initializer = expression_stmt();
        }


        std::unique_ptr<IExpr> condition{ nullptr };
        if (!state_.check(TokenType::semicolon)) {
            condition = expression();
        } else {
            condition = std::make_unique<LiteralExpr>(
                Token{
                    TokenType::kw_true, "true",
                    state_.current()->line, true
                }
            );
        }
        try_consume_semicolon();


        std::unique_ptr<IExpr> increment{ nullptr };
        if (!state_.check(TokenType::rparen)) {
            increment = expression();
        }

        try_consume(
            TokenType::rparen, ParserError::missing_closing_paren
        );


        /*

        { // full_body
            initializer;
            while (condition) { // full_stmts
                stmt;
                increment;
            }
        }

        */

        auto stmt = statement();

        std::vector<std::unique_ptr<IStmt>> full_body;

        if (initializer) {
            full_body.emplace_back(std::move(initializer));
        }

        if (increment) {
            std::vector<std::unique_ptr<IStmt>> full_stmts;
            full_stmts.emplace_back(std::move(stmt));
            full_stmts.emplace_back(std::make_unique<ExpressionStmt>(std::move(increment)));
            stmt = std::make_unique<BlockStmt>(std::move(full_stmts));
        }

        auto loop = std::make_unique<WhileStmt>(
            std::move(condition),
            std::move(stmt)
        );

        full_body.emplace_back(std::move(loop));

        return std::make_unique<BlockStmt>(
            std::move(full_body)
        );
    }

    std::unique_ptr<IStmt> while_stmt() {
        try_consume(
            TokenType::lparen, ParserError::missing_opening_paren
        );

        auto condition = expression();

        try_consume(
            TokenType::rparen, ParserError::missing_closing_paren
        );

        return std::make_unique<WhileStmt>(
            std::move(condition),
            statement()
        );
    }


    std::vector<std::unique_ptr<IStmt>> block() {
        std::vector<std::unique_ptr<IStmt>> stmts;

        while (!state_.is_eof() && !state_.check(TokenType::rbrace)) {
            stmts.emplace_back(declaration());
        }

        try_consume(
            TokenType::rbrace, ParserError::missing_closing_brace
        );

        return stmts;
    }

    std::unique_ptr<IStmt> block_stmt() {
        return std::make_unique<BlockStmt>(block());
    }

    std::unique_ptr<IStmt> print_stmt() {
        auto expr = expression();

        try_consume_semicolon();

        return std::make_unique<PrintStmt>(std::move(expr));
    }

    std::unique_ptr<IStmt> expression_stmt() {
        auto expr = expression();

        try_consume_semicolon();

        return std::make_unique<ExpressionStmt>(std::move(expr));
    }





    std::unique_ptr<IExpr> expression() {
        return assignment_expr();
    }

    std::unique_ptr<IExpr> assignment_expr() {
        auto expr = or_expr();

        using enum TokenType;
        if (state_.match(eq)) {
            Token op{ state_.peek_previous() };
            auto rvalue = assignment_expr();

            auto vexpr = dynamic_cast<VariableExpr*>(expr.get());
            if (vexpr) {
                return std::make_unique<AssignExpr>(
                    vexpr->identifier, std::move(op), std::move(rvalue)
                );
            } else {
                const Token& primary{ expr->accept(ExprGetPrimaryTokenVisitor{}) };
                report_error(
                    ParserError::invalid_assignment_target,
                    primary,
                    primary.lexeme
                );
            }
        }

        return expr;
    }

    std::unique_ptr<IExpr> or_expr() {
        auto expr = and_expr();

        using enum TokenType;
        while (state_.match(kw_or)) {
            Token op{ state_.peek_previous() };
            expr = std::make_unique<LogicalExpr>(
                op, std::move(expr), and_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<IExpr> and_expr() {
        auto expr = comparison_expr();

        using enum TokenType;
        while (state_.match(kw_and)) {
            Token op{ state_.peek_previous() };
            expr = std::make_unique<LogicalExpr>(
                op, std::move(expr), comparison_expr()
            );
        }

        return expr;
    }



    std::unique_ptr<IExpr> equality_expr() {
        auto expr = comparison_expr();

        using enum TokenType;
        while (state_.match(bang_eq, eq_eq)) {
            Token op{ state_.peek_previous() };
            expr = std::make_unique<BinaryExpr>(
                op, std::move(expr), comparison_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<IExpr> comparison_expr() {
        auto expr = term_expr();

        using enum TokenType;
        while (state_.match(greater, greater_eq, less, less_eq)) {
            Token op{ state_.peek_previous() };
            expr = std::make_unique<BinaryExpr>(
                op, std::move(expr), term_expr()
            );

        }

        return expr;
    }

    std::unique_ptr<IExpr> term_expr() {
        auto expr = factor_expr();

        using enum TokenType;
        while (state_.match(plus, minus)) {
            Token op{ state_.peek_previous() };
            expr = std::make_unique<BinaryExpr>(
                op, std::move(expr), factor_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<IExpr> factor_expr() {
        auto expr = unary_expr();

        using enum TokenType;
        while (state_.match(star, slash)) {
            Token op{ state_.peek_previous() };
            expr = std::make_unique<BinaryExpr>(
                op, std::move(expr), unary_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<IExpr> unary_expr() {

        using enum TokenType;
        if (state_.match(bang, minus, plus)) {
            Token op{ state_.peek_previous() };
            return std::make_unique<UnaryExpr>(
                op, unary_expr()
            );
        } else {
            return primary_expr();
        }
    }

    std::unique_ptr<IExpr> primary_expr() {
        using enum TokenType;
        if (state_.match(string, number, kw_true, kw_false, kw_nil)) {

            return std::make_unique<LiteralExpr>(
                state_.peek_previous()
            );
        } else if (state_.match(lparen)) {

            auto expr = expression();

            try_consume(rparen, ParserError::missing_closing_paren);

            return std::make_unique<GroupedExpr>(
                std::move(expr)
            );
        } else if (state_.match(identifier)) {
            return std::make_unique<VariableExpr>(
                state_.peek_previous()
            );
        }

        report_error_and_abort(ParserError::unknown_primary_expression);
        return { nullptr };
    }





    const Token& try_consume(TokenType expected, ParserError fail_error) {
        if (!state_.match(expected)) {
            report_error_and_abort(fail_error);
        }
        return state_.peek_previous();
    }

    const Token& try_consume_semicolon() {
        return try_consume(
            TokenType::semicolon, ParserError::missing_semicolon
        );
    }





    void synchronize_on_next_statement() {

        using enum TokenType;

        while(!state_.is_eof()) {

            const Token& prev{ state_.advance() };
            if (prev.type == semicolon) {
                return;
            }

            switch (state_.peek().type) {
                case kw_class:
                case kw_fun:
                case kw_var:
                case kw_for:
                case kw_if:
                case kw_while:
                case kw_print:
                case kw_return:
                    return;
                default:
                    break;
            }
        }

    }





    void abort_parsing_by_throwing_last_error() noexcept(false) {
        throw err_.get_parser_errors().back();
    }


    void report_error(ParserError type, std::string_view details = "") {
        err_.parser_error(type, state_.peek(), details);
    }

    void report_error(ParserError type, const Token& token, std::string_view details = "") {
        err_.parser_error(type, token, details);
    }


    void report_error_and_abort(ParserError type, std::string_view details = "") {
        report_error(type, details);
        abort_parsing_by_throwing_last_error();
    }
};


