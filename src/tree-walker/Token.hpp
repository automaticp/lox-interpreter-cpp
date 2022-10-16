#pragma once
#include "TokenType.hpp"
#include "LiteralValue.hpp"
#include <optional>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>
#include <cassert>

class Token {
public:
    const TokenType type;
    const std::string lexeme;
    const size_t line;
    const std::optional<LiteralValue> literal;

    Token(TokenType type, std::string_view lexeme, size_t line, std::optional<LiteralValue> literal = std::nullopt) :
        type{ type }, lexeme{ lexeme }, line{ line }, literal{ std::move(literal) } {}

    std::string info() const {
        std::stringstream ss;
        ss << type;
        std::string result;
        ss >> result;
        result = '[' + result + ']' + " \"" + lexeme + '\"';
        if (literal.has_value()) {
            result += " (" + std::visit(LiteralToStringVisitor{}, literal.value()) + ')';
        }
        return result;
    }

    bool operator==(const Token& other) const noexcept {
        bool is_eq{
            type == other.type &&
            lexeme == other.lexeme &&
            line == other.line
        };
        return is_eq;
    }

    bool operator!=(const Token& other) const noexcept {
        return !(*this == other);
    }

    operator const LiteralValue& () const {
        assert(literal.has_value());
        return literal.value();
    }
    operator TokenType() const { return type; }

};
