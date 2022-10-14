#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "Errors.hpp"
#include "Token.hpp"
#include <fmt/format.h>

class ErrorReporter {
private:
    std::vector<ContextError> context_errs_;
    std::vector<ScannerError> scanner_errs_;
    std::vector<ParserError> parser_errs_;

public:
    bool had_context_errors() const noexcept {
        return !context_errs_.empty();
    }

    bool had_scanner_errors() const noexcept {
        return !scanner_errs_.empty();
    }

    bool had_parser_errors() const noexcept {
        return !parser_errs_.empty();
    }

    bool had_errors() const noexcept {
        return had_context_errors() || had_scanner_errors() || had_parser_errors();
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


    virtual ~ErrorReporter() = default;

protected:
    virtual void report_context_error(ContextError type, std::string_view details = "") = 0;

    virtual void report_scanner_error(ScannerError type, size_t line, std::string_view details = "") = 0;

    virtual void report_parser_error(ParserError type, const Token& token, std::string_view details = "") = 0;

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
        return (details.size() < 16 ? ": " : ":\n    ");
    }
};
