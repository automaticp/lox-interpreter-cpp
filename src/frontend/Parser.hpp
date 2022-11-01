#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "ExprVisitors.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "TokenType.hpp"
#include <concepts>
#include <vector>
#include <cassert>
#include <concepts>
#include <memory>
#include <utility>
#include <span>


class Parser {
private:
    class ParserState {
    public:
        using iter_t = std::vector<Token>::const_iterator;
    private:
        iter_t beg_;
        iter_t cur_;
        iter_t end_;

    public:
        ParserState() = default;

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


    ErrorReporter& err_;
    std::vector<std::unique_ptr<IStmt>> statements_;

    ParserState state_;


    void prepare_tokens(const std::vector<Token>& new_tokens) {
        state_ = { new_tokens.begin(), new_tokens.end() };
    }


public:
    Parser(ErrorReporter& err) : err_{ err } {}

    // returns a view of new statements
    std::span<std::unique_ptr<IStmt>>
    parse_tokens(const std::vector<Token>& tokens) {
        prepare_tokens(tokens);

        size_t num_stmts_before = statements_.size();

        while (!state_.is_eof()) {
            try {
                statements_.emplace_back(declaration());
            } catch (ParserError) {
                synchronize_on_next_statement();
            }
        }

        return {
            statements_.begin() + num_stmts_before,
            statements_.size() - num_stmts_before
        };
    }

    const std::vector<std::unique_ptr<IStmt>>& peek_result() const {
        assert(is_eof());
        return statements_;
    }

    [[nodiscard]] std::vector<std::unique_ptr<IStmt>> get_result() {
        assert(is_eof());
        state_.reset();
        return std::move(statements_);
    }

    bool is_eof() const noexcept {
        return state_.is_eof();
    }




private:
    std::unique_ptr<IStmt> declaration() {
        if (state_.match(TokenType::kw_var)) {
            return var_decl();
        } else if (state_.match(TokenType::kw_fun)) {
            return fun_decl();
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
            init = IExpr::make_unique<LiteralExpr>(
                Token{TokenType::kw_nil, "nil", state_.current()->line, Nil{}}
            );
        }

        try_consume_semicolon();
        return IStmt::make_unique<VarStmt>(id, std::move(init));
    }

    std::unique_ptr<IStmt> fun_decl() {
        using enum TokenType;

        const Token& id = try_consume(
            identifier, ParserError::expected_identifier
        );

        try_consume(
            lparen, ParserError::missing_opening_paren
        );

        std::vector<Token> params;
        if (!state_.check(TokenType::rparen)) {
            do {
                params.emplace_back(
                    try_consume(
                        identifier,
                        ParserError::expected_identifier
                    )
                );
            } while (state_.match(comma));
        }

        try_consume(
            rparen, ParserError::missing_closing_paren
        );


        try_consume(
            lbrace, ParserError::missing_opening_brace
        );

        return IStmt::make_unique<FunStmt>(
            id,
            std::move(params),
            block()
        );
    }

    std::unique_ptr<IStmt> statement() {
        if (state_.match(TokenType::kw_if)) {
            return if_stmt();
        } else if (state_.match(TokenType::kw_return)){
            return return_stmt();
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

        return IStmt::make_unique<IfStmt>(
            std::move(condition),
            std::move(then_branch),
            std::move(else_branch)
        );
    }

    std::unique_ptr<IStmt> return_stmt() {
        const Token& keyword{ state_.peek_previous() };

        std::unique_ptr<IExpr> value;
        if (!state_.check(TokenType::semicolon)) {
            value = expression();
        } else {
            value = IExpr::make_unique<LiteralExpr>(
                Token{ TokenType::kw_nil, "nil", state_.current()->line, Nil{} }
            );
        }

        try_consume_semicolon();

        return IStmt::make_unique<ReturnStmt>(
            keyword, std::move(value)
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
            condition = IExpr::make_unique<LiteralExpr>(
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
            full_stmts.emplace_back(IStmt::make_unique<ExpressionStmt>(std::move(increment)));
            stmt = IStmt::make_unique<BlockStmt>(std::move(full_stmts));
        }

        auto loop = IStmt::make_unique<WhileStmt>(
            std::move(condition),
            std::move(stmt)
        );

        full_body.emplace_back(std::move(loop));

        return IStmt::make_unique<BlockStmt>(
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

        return IStmt::make_unique<WhileStmt>(
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
        return IStmt::make_unique<BlockStmt>(block());
    }

    std::unique_ptr<IStmt> print_stmt() {
        auto expr = expression();

        try_consume_semicolon();

        return IStmt::make_unique<PrintStmt>(std::move(expr));
    }

    std::unique_ptr<IStmt> expression_stmt() {
        auto expr = expression();

        try_consume_semicolon();

        return IStmt::make_unique<ExpressionStmt>(std::move(expr));
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

            if (expr->is<VariableExpr>()) {
                return IExpr::make_unique<AssignExpr>(
                    expr->as<VariableExpr>().identifier, std::move(op), std::move(rvalue)
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
            expr = IExpr::make_unique<LogicalExpr>(
                op, std::move(expr), and_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<IExpr> and_expr() {
        auto expr = equality_expr();

        using enum TokenType;
        while (state_.match(kw_and)) {
            Token op{ state_.peek_previous() };
            expr = IExpr::make_unique<LogicalExpr>(
                op, std::move(expr), equality_expr()
            );
        }

        return expr;
    }



    std::unique_ptr<IExpr> equality_expr() {
        auto expr = comparison_expr();

        using enum TokenType;
        while (state_.match(bang_eq, eq_eq)) {
            Token op{ state_.peek_previous() };
            expr = IExpr::make_unique<BinaryExpr>(
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
            expr = IExpr::make_unique<BinaryExpr>(
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
            expr = IExpr::make_unique<BinaryExpr>(
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
            expr = IExpr::make_unique<BinaryExpr>(
                op, std::move(expr), unary_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<IExpr> unary_expr() {

        using enum TokenType;
        if (state_.match(bang, minus, plus)) {
            Token op{ state_.peek_previous() };
            return IExpr::make_unique<UnaryExpr>(
                op, unary_expr()
            );
        } else {
            return call_expr();
        }
    }

    std::unique_ptr<IExpr> call_expr() {
        auto expr = primary_expr();

        while (true) {
            if (state_.match(TokenType::lparen)) {
                expr = parse_call_expr(std::move(expr));
            } else {
                break;
            }
        }

        return expr;
    }

    std::unique_ptr<IExpr>
    parse_call_expr(std::unique_ptr<IExpr> callee) {
        std::vector<std::unique_ptr<IExpr>> args;

        if (!state_.check(TokenType::rparen)) {
            do {
                args.emplace_back(expression());
            } while (state_.match(TokenType::comma));
        }

        const Token& closing_paren = try_consume(
            TokenType::rparen, ParserError::missing_closing_paren
        );

        return IExpr::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            closing_paren
        );
    }


    std::unique_ptr<IExpr> primary_expr() {
        using enum TokenType;
        if (state_.match(string, number, kw_true, kw_false, kw_nil)) {

            return IExpr::make_unique<LiteralExpr>(
                state_.peek_previous()
            );
        } else if (state_.match(lparen)) {

            auto expr = expression();

            try_consume(rparen, ParserError::missing_closing_paren);

            return IExpr::make_unique<GroupedExpr>(
                std::move(expr)
            );
        } else if (state_.match(identifier)) {
            return IExpr::make_unique<VariableExpr>(
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

