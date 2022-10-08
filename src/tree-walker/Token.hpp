#pragma once
#include "TokenType.hpp"
#include "LiteralValue.hpp"
#include <optional>
#include <string>
#include <sstream>

class Token {
public:
    TokenType type;
    std::string lexeme;
    size_t line;
    std::optional<LiteralValue> literal{ std::nullopt };

    std::string info() const {
        std::stringstream ss;
        ss << type;
        std::string result;
        ss >> result;
        return result + " \"" + lexeme + '\"';
    }
};
