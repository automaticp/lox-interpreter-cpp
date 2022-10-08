#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <cassert>
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

        iter_t current() const noexcept { return cur_; }

        char peek() const noexcept {
            assert(!is_end());
            return *cur_;
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

    ScannerState state_{ source_.cbegin(), source_.cend() };

public:
    Scanner(std::string_view source, ErrorReporter& err) : source_{ source }, err_{ err } {}

    const std::vector<Token>& scan_tokens() {

        while (!state_.is_end()) {
            state_.new_token();
            scan_token();
        }
    }

    std::string::const_iterator scan_token() noexcept {

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

            default: err_.error(state_.line(), std::string("Unexpected character: ") + c); break;
        }

    }

    void add_token(TokenType type) {
        tokens_.emplace_back(type, state_.lexeme(), state_.line());
    }

    ErrorReporter& get_error_reporter() noexcept {
        return err_;
    }

};
