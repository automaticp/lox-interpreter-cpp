#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <cassert>
#include <utility>
#include "Token.hpp"
#include "ErrorReporter.hpp"

class Scanner {
private:
    std::string source_;
    ErrorReporter& err_;
    std::vector<Token> tokens_;

    class ScannerState {
    public:
        using iter_t = std::string::const_iterator;
    private:
        const iter_t beg_;
        iter_t cur_;
        iter_t token_beg_;
        const iter_t end_;
        size_t line_;

    public:
        ScannerState(iter_t beg, iter_t end, size_t start_line) :
            beg_{ beg }, cur_{ beg }, end_{ end },
            line_{ start_line }
        {}

        bool is_end() const noexcept { return cur_ == end_; }

        bool is_next_end() const noexcept { return cur_ + 1 == end_; }

        iter_t begin() const noexcept { return beg_; }

        iter_t end() const noexcept { return end_; }

        iter_t current() const noexcept { return cur_; }

        iter_t token_begin() const noexcept { return token_beg_; }

        char peek() const noexcept {
            assert(!is_end());
            return *cur_;
        }

        char peek_next() const noexcept {
            assert(!is_end());
            assert(current() + 1 != end_);
            return *(current() + 1);
        }

        char advance() noexcept {
            assert(!is_end());
            return *cur_++;
        }

        bool match(char expected) noexcept {
            if (is_end()) return false;

            if (peek() != expected) {
                return false;
            } else {
                advance();
                return true;
            }
        }

        size_t line() const noexcept { return line_; }
        void add_line() noexcept { ++line_; }

        // Call each time before scanning the next token
        void new_token() noexcept { token_beg_ = cur_; }

        std::string_view lexeme() const noexcept {
            return { token_beg_, cur_ };
        }

    };

    ScannerState state_{ source_.cbegin(), source_.cend(), 0 };

public:
    Scanner(std::string_view source, ErrorReporter& err) : source_{ source }, err_{ err } {}

    const std::vector<Token>& scan_tokens() {

        while (!state_.is_end()) {
            state_.new_token();
            scan_token();
        }

        return tokens_;
    }


private:
    void scan_token() noexcept {

        using enum TokenType;

        char c{ state_.advance() };
        switch (c) {
            case '(': add_token(lparen); break;
            case ')': add_token(rparen); break;
            case '{': add_token(lbrace); break;
            case '}': add_token(rbrace); break;
            case ',': add_token(comma); break;
            case '.': add_token(dot); break;
            case '-': add_token(minus); break;
            case '+': add_token(plus); break;
            case ';': add_token(semicolon); break;
            case '*': add_token(star); break;
            case '!': add_token(state_.match('=') ? bang_eq : bang); break;
            case '=': add_token(state_.match('=') ? eq_eq : eq); break;
            case '<': add_token(state_.match('=') ? less_eq : less); break;
            case '>': add_token(state_.match('=') ? greater_eq : greater); break;
            case '/':
                if (state_.match('/')) {
                    while (!state_.is_end() && state_.peek() != '\n') {
                        state_.advance();
                    }
                } else {
                    add_token(slash);
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                state_.add_line();
                break;
            case '"':
                add_string_literal_token();
                break;

            default:
                if (is_digit(c)) {
                    add_number_literal_token();
                } else if (is_alpha(c)) {
                    add_identifier_token();
                } else {
                    report_error(std::string("Unexpected character: ") + c);
                }
                break;
        }

    }

    void add_token(TokenType type) {
        tokens_.emplace_back(type, state_.lexeme(), state_.line());
    }

    void add_token(TokenType type, LiteralValue&& literal) {
        tokens_.emplace_back(type, state_.lexeme(), state_.line(), std::move(literal));
    }

    void add_string_literal_token() {
        while (!state_.is_end() && state_.peek() != '"') {
            if (state_.peek() == '\n') state_.add_line();
            state_.advance();
        }

        if (state_.is_end()) {
            report_error("Unterminated string literal.");
            return;
        }

        state_.advance(); // past the closing "

        std::string_view quoted_literal = state_.lexeme();

        add_token(
            TokenType::string,
            std::string(quoted_literal.substr(1, quoted_literal.size() - 2))
        );
    }


    void add_number_literal_token() {
        while (!state_.is_end() && is_digit(state_.peek())) {
            state_.advance();
        }

        if (!state_.is_end() && state_.peek() == '.' &&
            !state_.is_next_end()) {

            if (is_digit(state_.peek_next())) {
                state_.advance(); // past the .

                while (!state_.is_end() && is_digit(state_.peek())) {
                    state_.advance();
                }
            } else {
                report_error("Unterminated number literal.");
                return;
            }
        }

        add_token(
            TokenType::number,
            std::stod(std::string(state_.lexeme()))
        );
    }

    void add_identifier_token() {
        while (!state_.is_end() && is_alphanum(state_.peek())) {
            state_.advance();
        }

        auto it = detail::keyword_map.find(std::string(state_.lexeme()));
        if (it != detail::keyword_map.end()) {
            add_token(it->second);
        } else {
            add_token(TokenType::identifier);
        }
    }

    void report_error(std::string_view message) {
        err_.error(state_.line(), message);
    }


    ErrorReporter& get_error_reporter() noexcept {
        return err_;
    }

private:
    static bool is_digit(char c) noexcept {
        return c >= '0' && c <= '9';
    }

    static bool is_alpha(char c) noexcept {
        return (
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c == '_')
        );
    }

    static bool is_alphanum(char c) noexcept {
        return is_alpha(c) || is_digit(c);
    }

};
