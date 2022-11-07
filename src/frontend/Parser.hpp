#pragma once
#include "ErrorReporter.hpp"
#include "FrontendErrors.hpp"
#include "ErrorSender.hpp"
#include "CommonVisitors.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include "TokenType.hpp"
#include "TokenIterator.hpp"
#include <concepts>
#include <vector>
#include <cassert>
#include <concepts>
#include <memory>
#include <utility>
#include <span>


class Parser : private ErrorSender<ParserError> {
private:
    std::vector<std::unique_ptr<Stmt>> statements_;
    TokenIterator<std::vector<Token>::const_iterator> state_;

    void prepare_tokens(const std::vector<Token>& new_tokens) {
        state_.reset(new_tokens.begin(), new_tokens.end());
    }


public:
    Parser(ErrorReporter& err) : ErrorSender{ err } {}

    // returns a view of new statements
    std::span<std::unique_ptr<Stmt>>
    parse_tokens(const std::vector<Token>& tokens) {
        prepare_tokens(tokens);

        size_t num_stmts_before = statements_.size();

        while (!state_.is_eof()) {
            try {
                statements_.emplace_back(declaration());
            } catch (ParserError::Type) {
                synchronize_on_next_statement();
            }
        }

        return {
            statements_.begin() + num_stmts_before,
            statements_.size() - num_stmts_before
        };
    }

    const std::vector<std::unique_ptr<Stmt>>& peek_result() const {
        assert(is_eof());
        return statements_;
    }

    [[nodiscard]] std::vector<std::unique_ptr<Stmt>> get_result() {
        assert(is_eof());
        state_.reset();
        return std::move(statements_);
    }

    bool is_eof() const noexcept {
        return state_.is_eof();
    }




private:
    std::unique_ptr<Stmt> declaration() {
        // Skip import statements
        while (state_.match(TokenType::kw_import)) {
            skip_import();
        }

        if (state_.match(TokenType::kw_var)) {
            return var_decl();
        } else if (state_.match(TokenType::kw_fun)) {
            return fun_decl();
        } else {
            return statement();
        }
    }

    void skip_import() {
        bool match_succeded{ state_.match(TokenType::string) };
        assert(match_succeded &&
            "If this failed, then the Importer didn't do it's job");
        try_consume_semicolon();
    }

    std::unique_ptr<Stmt> var_decl() {
        const Token& id = try_consume(
            TokenType::identifier, ParserError::Type::expected_identifier
        );

        // FIXME: could be LiteralExpr(nil) or its own NullExpr;
        std::unique_ptr<Expr> init;
        if (state_.match(TokenType::eq)) {
            init = expression();
        } else {
            init = Expr::make_unique<LiteralExpr>(
                Token{TokenType::kw_nil, "nil", state_.current()->line, Nil{}}
            );
        }

        try_consume_semicolon();
        return Stmt::make_unique<VarStmt>(id, std::move(init));
    }

    std::unique_ptr<Stmt> fun_decl() {
        using enum TokenType;

        const Token& id = try_consume(
            identifier, ParserError::Type::expected_identifier
        );

        try_consume(
            lparen, ParserError::Type::missing_opening_paren
        );

        std::vector<Token> params;
        if (!state_.check(TokenType::rparen)) {
            do {
                params.emplace_back(
                    try_consume(
                        identifier,
                        ParserError::Type::expected_identifier
                    )
                );
            } while (state_.match(comma));
        }

        try_consume(
            rparen, ParserError::Type::missing_closing_paren
        );


        try_consume(
            lbrace, ParserError::Type::missing_opening_brace
        );

        return Stmt::make_unique<FunStmt>(
            id,
            std::move(params),
            block()
        );
    }

    std::unique_ptr<Stmt> statement() {
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

    std::unique_ptr<Stmt> if_stmt() {
        try_consume(
            TokenType::lparen, ParserError::Type::missing_opening_paren
        );

        auto condition = expression();

        try_consume(
            TokenType::rparen, ParserError::Type::missing_closing_paren
        );

        auto then_branch = statement();
        std::unique_ptr<Stmt> else_branch{ nullptr };
        if (state_.match(TokenType::kw_else)) {
            else_branch = statement();
        }

        return Stmt::make_unique<IfStmt>(
            std::move(condition),
            std::move(then_branch),
            std::move(else_branch)
        );
    }

    std::unique_ptr<Stmt> return_stmt() {
        const Token& keyword{ state_.peek_previous() };

        std::unique_ptr<Expr> value;
        if (!state_.check(TokenType::semicolon)) {
            value = expression();
        } else {
            value = Expr::make_unique<LiteralExpr>(
                Token{ TokenType::kw_nil, "nil", state_.current()->line, Nil{} }
            );
        }

        try_consume_semicolon();

        return Stmt::make_unique<ReturnStmt>(
            keyword, std::move(value)
        );
    }


    std::unique_ptr<Stmt> for_stmt() {
        try_consume(
            TokenType::lparen, ParserError::Type::missing_opening_paren
        );


        std::unique_ptr<Stmt> initializer{ nullptr };
        if (state_.match(TokenType::semicolon)) {
            /* nothing, already null */
        } else if (state_.match(TokenType::kw_var)) {
            initializer = var_decl();
        } else {
            initializer = expression_stmt();
        }


        std::unique_ptr<Expr> condition{ nullptr };
        if (!state_.check(TokenType::semicolon)) {
            condition = expression();
        } else {
            condition = Expr::make_unique<LiteralExpr>(
                Token{
                    TokenType::kw_true, "true",
                    state_.current()->line, true
                }
            );
        }
        try_consume_semicolon();


        std::unique_ptr<Expr> increment{ nullptr };
        if (!state_.check(TokenType::rparen)) {
            increment = expression();
        }

        try_consume(
            TokenType::rparen, ParserError::Type::missing_closing_paren
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

        std::vector<std::unique_ptr<Stmt>> full_body;

        if (initializer) {
            full_body.emplace_back(std::move(initializer));
        }

        if (increment) {
            std::vector<std::unique_ptr<Stmt>> full_stmts;
            full_stmts.emplace_back(std::move(stmt));
            full_stmts.emplace_back(Stmt::make_unique<ExpressionStmt>(std::move(increment)));
            stmt = Stmt::make_unique<BlockStmt>(std::move(full_stmts));
        }

        auto loop = Stmt::make_unique<WhileStmt>(
            std::move(condition),
            std::move(stmt)
        );

        full_body.emplace_back(std::move(loop));

        return Stmt::make_unique<BlockStmt>(
            std::move(full_body)
        );
    }

    std::unique_ptr<Stmt> while_stmt() {
        try_consume(
            TokenType::lparen, ParserError::Type::missing_opening_paren
        );

        auto condition = expression();

        try_consume(
            TokenType::rparen, ParserError::Type::missing_closing_paren
        );

        return Stmt::make_unique<WhileStmt>(
            std::move(condition),
            statement()
        );
    }


    std::vector<std::unique_ptr<Stmt>> block() {
        std::vector<std::unique_ptr<Stmt>> stmts;

        while (!state_.is_eof() && !state_.check(TokenType::rbrace)) {
            stmts.emplace_back(declaration());
        }

        try_consume(
            TokenType::rbrace, ParserError::Type::missing_closing_brace
        );

        return stmts;
    }

    std::unique_ptr<Stmt> block_stmt() {
        return Stmt::make_unique<BlockStmt>(block());
    }

    std::unique_ptr<Stmt> print_stmt() {
        auto expr = expression();

        try_consume_semicolon();

        return Stmt::make_unique<PrintStmt>(std::move(expr));
    }

    std::unique_ptr<Stmt> expression_stmt() {
        auto expr = expression();

        try_consume_semicolon();

        return Stmt::make_unique<ExpressionStmt>(std::move(expr));
    }





    std::unique_ptr<Expr> expression() {
        return assignment_expr();
    }

    std::unique_ptr<Expr> assignment_expr() {
        auto expr = or_expr();

        using enum TokenType;
        if (state_.match(eq)) {
            Token op{ state_.peek_previous() };
            auto rvalue = assignment_expr();

            if (expr->is<VariableExpr>()) {
                return Expr::make_unique<AssignExpr>(
                    expr->as<VariableExpr>().identifier, std::move(op), std::move(rvalue)
                );
            } else {
                const Token& primary{ expr->accept(ExprGetPrimaryTokenVisitor{}) };
                report_error(
                    ParserError::Type::invalid_assignment_target,
                    primary,
                    primary.lexeme
                );
            }
        }

        return expr;
    }

    std::unique_ptr<Expr> or_expr() {
        auto expr = and_expr();

        using enum TokenType;
        while (state_.match(kw_or)) {
            Token op{ state_.peek_previous() };
            expr = Expr::make_unique<LogicalExpr>(
                op, std::move(expr), and_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<Expr> and_expr() {
        auto expr = equality_expr();

        using enum TokenType;
        while (state_.match(kw_and)) {
            Token op{ state_.peek_previous() };
            expr = Expr::make_unique<LogicalExpr>(
                op, std::move(expr), equality_expr()
            );
        }

        return expr;
    }



    std::unique_ptr<Expr> equality_expr() {
        auto expr = comparison_expr();

        using enum TokenType;
        while (state_.match(bang_eq, eq_eq)) {
            Token op{ state_.peek_previous() };
            expr = Expr::make_unique<BinaryExpr>(
                op, std::move(expr), comparison_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<Expr> comparison_expr() {
        auto expr = term_expr();

        using enum TokenType;
        while (state_.match(greater, greater_eq, less, less_eq)) {
            Token op{ state_.peek_previous() };
            expr = Expr::make_unique<BinaryExpr>(
                op, std::move(expr), term_expr()
            );

        }

        return expr;
    }

    std::unique_ptr<Expr> term_expr() {
        auto expr = factor_expr();

        using enum TokenType;
        while (state_.match(plus, minus)) {
            Token op{ state_.peek_previous() };
            expr = Expr::make_unique<BinaryExpr>(
                op, std::move(expr), factor_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<Expr> factor_expr() {
        auto expr = unary_expr();

        using enum TokenType;
        while (state_.match(star, slash)) {
            Token op{ state_.peek_previous() };
            expr = Expr::make_unique<BinaryExpr>(
                op, std::move(expr), unary_expr()
            );
        }

        return expr;
    }

    std::unique_ptr<Expr> unary_expr() {

        using enum TokenType;
        if (state_.match(bang, minus, plus)) {
            Token op{ state_.peek_previous() };
            return Expr::make_unique<UnaryExpr>(
                op, unary_expr()
            );
        } else {
            return call_expr();
        }
    }

    std::unique_ptr<Expr> call_expr() {
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

    std::unique_ptr<Expr>
    parse_call_expr(std::unique_ptr<Expr> callee) {
        std::vector<std::unique_ptr<Expr>> args;

        if (!state_.check(TokenType::rparen)) {
            do {
                args.emplace_back(expression());
            } while (state_.match(TokenType::comma));
        }

        const Token& closing_paren = try_consume(
            TokenType::rparen, ParserError::Type::missing_closing_paren
        );

        return Expr::make_unique<CallExpr>(
            std::move(callee),
            std::move(args),
            closing_paren
        );
    }


    std::unique_ptr<Expr> primary_expr() {
        using enum TokenType;
        if (state_.match(string, number, kw_true, kw_false, kw_nil)) {

            return Expr::make_unique<LiteralExpr>(
                state_.peek_previous()
            );
        } else if (state_.match(lparen)) {

            auto expr = expression();

            try_consume(rparen, ParserError::Type::missing_closing_paren);

            return Expr::make_unique<GroupedExpr>(
                std::move(expr)
            );
        } else if (state_.match(identifier)) {
            return Expr::make_unique<VariableExpr>(
                state_.peek_previous()
            );
        }

        report_error_and_abort(ParserError::Type::unknown_primary_expression);
        return { nullptr };
    }





    const Token& try_consume(TokenType expected, ParserError::Type fail_error) {
        if (!state_.match(expected)) {
            report_error_and_abort(fail_error);
        }
        return state_.peek_previous();
    }

    const Token& try_consume_semicolon() {
        return try_consume(
            TokenType::semicolon, ParserError::Type::missing_semicolon
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
                case kw_import:
                    return;
                default:
                    break;
            }
        }

    }




    void abort_by_exception(ParserError::Type type) const noexcept(false) {
        throw type;
    }

    void report_error(ParserError::Type type, std::string_view details = "") {
        send_error(type, state_.peek(), std::string(details));
    }

    void report_error(ParserError::Type type, const Token& token, std::string_view details = "") {
        send_error(type, token, std::string(details));
    }


    void report_error_and_abort(ParserError::Type type, std::string_view details = "") {
        report_error(type, details);
        abort_by_exception(type);
    }
};


