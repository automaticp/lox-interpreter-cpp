#pragma once
#include "TokenType.hpp"
#include "LiteralValue.hpp"
#include <optional>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>

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
};
