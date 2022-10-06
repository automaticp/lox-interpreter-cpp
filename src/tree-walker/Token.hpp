#pragma once
#include "TokenType.hpp"
#include <string>
#include <sstream>

class Token {
public:
    TokenType type;
    std::string lexeme;
    size_t line;

    std::string info() const {
        std::stringstream ss;
        ss << type;
        std::string result;
        ss >> result;
        return result + " \"" + lexeme + '\"';
    }
};
