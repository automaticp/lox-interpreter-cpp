#pragma once
#include "Token.hpp"
#include <vector>
#include <concepts>


// A general purpose utility for walking a range of Tokens.
// (What once was a ParserState)

template<typename RandomIt>
class TokenIterator {
public:
    using iter_t = RandomIt;
private:
    iter_t beg_;
    iter_t cur_;
    iter_t end_;

public:
    TokenIterator() = default;

    TokenIterator(iter_t beg, iter_t end) :
        beg_{ beg }, cur_{ beg }, end_{ end } {}

    void reset() noexcept { cur_ = beg_; }

    void reset(iter_t beg, iter_t end) noexcept {
        beg_ = beg;
        end_ = end;
        reset();
    }

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
