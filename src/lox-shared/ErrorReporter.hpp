#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "Errors.hpp"
#include "ExprVisitors.hpp"
#include "StmtVisitors.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include <fmt/format.h>

class ErrorReporter {
private:
    std::vector<ContextError> context_errs_;
    std::vector<ScannerError> scanner_errs_;
    std::vector<ParserError> parser_errs_;
    std::vector<ResolverError> resolver_errs_;
    std::vector<InterpreterError> interpreter_errs_;

public:
    void reset() {
        context_errs_.clear();
        scanner_errs_.clear();
        parser_errs_.clear();
        resolver_errs_.clear();
        interpreter_errs_.clear();
    }

    bool had_context_errors() const noexcept {
        return !context_errs_.empty();
    }

    bool had_scanner_errors() const noexcept {
        return !scanner_errs_.empty();
    }

    bool had_parser_errors() const noexcept {
        return !parser_errs_.empty();
    }

    bool had_interpreter_errors() const noexcept {
        return !interpreter_errs_.empty();
    }

    bool had_resolver_errors() const noexcept {
        return !resolver_errs_.empty();
    }

    bool had_errors() const noexcept {
        return had_context_errors() || had_scanner_errors()
            || had_parser_errors() || had_interpreter_errors()
            || had_resolver_errors();
    }

    const std::vector<ContextError>&
    get_context_errors() const noexcept {
        return context_errs_;
    }

    const std::vector<ScannerError>&
    get_scanner_errors() const noexcept {
        return scanner_errs_;
    }

    const std::vector<ParserError>&
    get_parser_errors() const noexcept {
        return parser_errs_;
    }

    const std::vector<ResolverError>&
    get_resolver_errors() const noexcept {
        return resolver_errs_;
    }

    const std::vector<InterpreterError>&
    get_interpreter_errors() const noexcept {
        return interpreter_errs_;
    }


    void scanner_error(ScannerError type, size_t line, std::string_view details) {
        scanner_errs_.push_back(type);
        report_scanner_error(type, line, details);
    }

    void context_error(ContextError type, std::string_view details) {
        context_errs_.push_back(type);
        report_context_error(type, details);
    }

    void parser_error(ParserError type, const Token& token, std::string_view details) {
        parser_errs_.push_back(type);
        report_parser_error(type, token, details);
    }

    void resolver_error(ResolverError type, const Expr& expr, std::string_view details) {
        resolver_errs_.push_back(type);
        report_resolver_error(type, expr, details);
    }

    void resolver_error(ResolverError type, const Stmt& stmt, std::string_view details) {
        resolver_errs_.push_back(type);
        report_resolver_error(type, stmt, details);
    }

    void interpreter_error(InterpreterError type, const Expr& expr, std::string_view details) {
        interpreter_errs_.push_back(type);
        report_interpreter_error(type, expr, details);
    }


    virtual ~ErrorReporter() = default;

protected:
    virtual void report_context_error(ContextError type, std::string_view details = "") = 0;

    virtual void report_scanner_error(ScannerError type, size_t line, std::string_view details = "") = 0;

    virtual void report_parser_error(ParserError type, const Token& token, std::string_view details = "") = 0;

    virtual void report_resolver_error(ResolverError type, const Expr& expr, std::string_view details = "") = 0;
    virtual void report_resolver_error(ResolverError type, const Stmt& stmt, std::string_view details = "") = 0;

    virtual void report_interpreter_error(InterpreterError type, const Expr& expr, std::string_view details = "") = 0;
};


class StreamErrorReporter : public ErrorReporter {
private:
    std::ostream& os_;
public:
    StreamErrorReporter(std::ostream& os) : os_{ os } {}

protected:
    void report_context_error(ContextError type, std::string_view details) override {
        os_ << fmt::format(
            "[Error @Context]:\n{:s}{:s}\n",
            to_error_message(type), details_tail(details)
        );
    }


    void report_scanner_error(ScannerError type, size_t line, std::string_view details) override {
        os_ << fmt::format(
            "[Error @Scanner] at line {:d}:\n{:s}{:s}\n",
            line, to_error_message(type), details_tail(details)
        );
    }


    void report_parser_error(ParserError type, const Token& token, std::string_view details) override {
        os_ << fmt::format(
            "[Error @Parser] at line {:d} token {:s}:\n{:s}{:s}\n",
            token.line, token.lexeme, to_error_message(type), details_tail(details)
        );
    }

    void report_resolver_error(ResolverError type, const Expr& expr, std::string_view details) override {
        const Token& primary{ expr.accept(ExprGetPrimaryTokenVisitor{}) };
        os_ << fmt::format(
            "[Error @Resolver] at line {:d} in {:s} ({:s}):\n{:s}{:s}\n",
            primary.line, expr.accept(ExprUserFriendlyNameVisitor{}), primary.lexeme,
            to_error_message(type), details_tail(details)
        );
    }

    void report_resolver_error(ResolverError type, const Stmt& stmt, std::string_view details) override {
        os_ << fmt::format(
            "[Error @Resolver] in {:s}:\n{:s}{:s}\n", // FIXME: how to get the line of the statement?
            stmt.accept(StmtUserFriendlyNameVisitor{}),
            to_error_message(type), details_tail(details)
        );
    }

    void report_interpreter_error(InterpreterError type, const Expr& expr, std::string_view details) override {
        const Token& primary{ expr.accept(ExprGetPrimaryTokenVisitor{}) };
        os_ << fmt::format(
            "[Error @Interpreter] at line {:d} in {:s} ({:s}):\n{:s}{:s}\n",
            primary.line, expr.accept(ExprUserFriendlyNameVisitor{}), primary.lexeme,
            to_error_message(type), details_tail(details)
        );
    }


private:
    static std::string details_tail(std::string_view details) {
        if (details.empty()) return ".";
        return fmt::format(
            "{:s}{:s}.",
            details_sep(details),
            details
        );
    }

    static std::string_view details_sep(std::string_view details) {
        constexpr size_t line_wrap_limit{ 16 }; // Chosen arbitrarily
        return (details.size() < line_wrap_limit ? ": " : ":\n    ");
    }
};
