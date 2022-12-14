#pragma once
#include "Token.hpp"
#include "TokenType.hpp"
#include "FrontendErrors.hpp"
#include "ErrorSender.hpp"
#include "ErrorReporter.hpp"
#include <filesystem>
#include <vector>
#include <string>
#include <string_view>
#include <cassert>
#include <utility>


class Scanner : private ErrorSender<ScannerError> {
private:
    class ScannerState {
    public:
        using iter_t = std::string::const_iterator;
    private:
        iter_t beg_;
        iter_t cur_;
        iter_t token_beg_;
        iter_t end_;
        SourceLocation location_{ 0, 0 };

    public:
        ScannerState() = default;

        ScannerState(iter_t beg, iter_t end, SourceLocation location) :
            beg_{ beg }, cur_{ beg }, end_{ end },
            location_{ std::move(location) }
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
            add_column();
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

        const SourceLocation& location() const noexcept { return location_; }


        void add_line() noexcept {
            ++location_.line;
            reset_column();
        }
        void add_column() noexcept {
            ++location_.column;
        }
        void reset_column() noexcept {
            location_.column = 1;
        }

        // Call each time before scanning the next token
        void new_token() noexcept { token_beg_ = cur_; }

        std::string_view lexeme() const noexcept {
            return { token_beg_, cur_ };
        }

    };

    std::vector<Token> tokens_;
    ScannerState state_;
    bool did_produce_error_{};

    void prepare_source(const std::string& text, const std::filesystem::path& file) {
        state_ = {
            text.cbegin(), text.cend(),
            SourceLocation{
                 1, 1,
                 std::make_shared<std::filesystem::path>(std::filesystem::canonical(file))
                }
        };
        did_produce_error_ = false;
    }

    void prepare_source(const std::string& text) {
        state_ = {
            text.cbegin(), text.cend(),
            { 1, 1 }
        };
        did_produce_error_ = false;
    }


public:
    Scanner(ErrorReporter& err) : ErrorSender{ err } {}

    [[nodiscard]]
    std::vector<Token> scan_tokens(const std::string& source_text, const std::filesystem::path& file) {
        // It's your job to check that 'source_text' comes from 'file'.
        // Otherwise, things can become awkward.

        prepare_source(source_text, file);

        while (!state_.is_end()) {
            state_.new_token();
            scan_token();
        }
        // Don't add eof here, add it manually
        // after resolving imports

        // We return the result even if the scanning
        // reported an error somewhere along the line,
        // since the tokens could still be useful
        // for debugging or other things.
        return std::move(tokens_);
    }

    [[nodiscard]]
    std::vector<Token> scan_tokens(const std::string& source_text) {
        prepare_source(source_text);
        while (!state_.is_end()) {
            state_.new_token();
            scan_token();
        }
        return std::move(tokens_);
    }

    bool has_failed() const noexcept { return did_produce_error_; }

    // Append a special symbol that tells the Parser
    // to stop parsing. Will preserve source location information.
    static void append_eof(std::vector<Token>& tokens) {
        SourceLocation location{0, 0};
        if (!tokens.empty()) {
            location = tokens.back().location();
            // FIXME: ermm, are we pointing at the beginning
            // or the end of the token? I think it was the end...
            // The next line is kina wrong then, but shouldn't
            // break anything.
            location.column += tokens.back().lexeme().size();
        }

        tokens.emplace_back(
            Token{
                TokenType::eof,
                to_lexeme(TokenType::eof),
                location,
                {}
            }
        );
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
                    skip_singleline_comment();
                } else if (state_.match('*')) {
                    skip_multiline_comment();
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
                    report_error(
                        ScannerError::Type::unexpected_character, std::string{c}
                    );
                }
                break;
        }

    }

    void skip_singleline_comment() {
        while (!state_.is_end() && state_.peek() != '\n') {
            state_.advance();
        }
    }

    void skip_multiline_comment() {
        while(!state_.is_end()) {

            switch (state_.advance()) {
                case '/':
                    if (state_.match('*')) {
                        skip_multiline_comment();
                    }
                    break;
                case '*':
                    if (state_.match('/')) {
                        return;
                    }
                    break;
                case '\n':
                    state_.add_line();
                    break;
                default:
                    break;
            }

        }
    }


    void add_token(TokenType type) {
        tokens_.emplace_back(type, std::string(state_.lexeme()), state_.location());
    }

    void add_token(TokenType type, LiteralValue&& literal) {
        tokens_.emplace_back(type, std::string(state_.lexeme()), state_.location(), std::move(literal));
    }

    void add_string_literal_token() {
        while (!state_.is_end() && state_.peek() != '"') {
            if (state_.peek() == '\n') state_.add_line();
            state_.advance();
        }

        if (state_.is_end()) {
            report_error(
                ScannerError::Type::unterminated_string_literal, state_.lexeme()
            );
            return;
        }

        state_.advance(); // past the closing "

        std::string_view quoted_literal = state_.lexeme();

        add_token(
            TokenType::string,
            String(quoted_literal.substr(1, quoted_literal.size() - 2))
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
                report_error(
                    ScannerError::Type::unterminated_number_literal, state_.lexeme()
                );
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
            if (it->second == TokenType::kw_true) {
                add_token(it->second, true);
            } else if (it->second == TokenType::kw_false) {
                add_token(it->second, false);
            } else if (it->second == TokenType::kw_nil) {
                add_token(it->second, Nil{});
            } else {
                add_token(it->second);
            }
        } else {
            add_token(TokenType::identifier);
        }
    }

    void report_error(ScannerError::Type type, std::string_view details = "") {
        did_produce_error_ = true; // FIXME: find a better way?
        send_error(
            type, state_.location(), std::string(details)
        );
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
