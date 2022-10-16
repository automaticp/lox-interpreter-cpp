#pragma once
#include <cstddef>
#include <iostream>
#include <array>
#include <string>
#include <unordered_map>
#include "Utils.hpp"

enum class TokenType : size_t {

    // Single-character
    lparen = 0, rparen, lbrace, rbrace,
    comma, dot, semicolon,
    minus, plus, slash, star,

    // Be Greedy
    bang, bang_eq,
    eq, eq_eq,
    greater, greater_eq,
    less, less_eq,

    //
    identifier,

    // Literals
    string, number,

    // Keywords
    kw_and, kw_class, kw_else, kw_false, kw_fun, kw_for,
    kw_if, kw_nil, kw_or, kw_print, kw_return, kw_super, kw_this,
    kw_true, kw_var, kw_while,

    //
    eof

};

namespace detail {

inline constexpr std::array token_type_names{

    "lparen", "rparen", "lbrace", "rbrace",
    "comma", "dot", "semicolon",
    "minus", "plus", "slash", "star",

    "bang", "bang_eq",
    "eq", "eq_eq",
    "greater", "greater_eq",
    "less", "less_eq",

    "identifier",

    "string", "number",

    "kw_and", "kw_class", "kw_else", "kw_false", "kw_fun", "kw_for",
    "kw_if", "kw_nil", "kw_or", "kw_print", "kw_return", "kw_super", "kw_this",
    "kw_true", "kw_var", "kw_while",

    "eof"
};

inline constexpr std::array token_type_lexemes{

    "(", ")", "{", "}",
    ",", ".", ";",
    "-", "+", "/", "*",

    "!", "!=",
    "=", "==",
    ">", ">=",
    "<", "<=",

    "?identifier?",

    "?string?", "?number?",

    "and", "class", "else", "false", "fun", "for",
    "if", "nil", "or", "print", "return", "super", "this",
    "true", "var", "while",

    "?eof?"
};


inline const std::unordered_map<std::string, TokenType> keyword_map{
    {"and",     TokenType::kw_and},
    {"class",   TokenType::kw_class},
    {"else",    TokenType::kw_else},
    {"false",   TokenType::kw_false},
    {"fun",     TokenType::kw_fun},
    {"for",     TokenType::kw_for},
    {"if",      TokenType::kw_if},
    {"nil",     TokenType::kw_nil},
    {"or",      TokenType::kw_or},
    {"print",   TokenType::kw_print},
    {"return",  TokenType::kw_return},
    {"super",   TokenType::kw_super},
    {"this",    TokenType::kw_this},
    {"true",    TokenType::kw_true},
    {"var",     TokenType::kw_var},
    {"while",   TokenType::kw_while},
};

} // namespace detail


inline std::string to_string(TokenType type) {
    return { detail::token_type_names[
        to_underlying(type)
    ] };
}

inline std::string to_lexeme(TokenType type) {
    return { detail::token_type_lexemes[
        to_underlying(type)
    ] };
}

inline std::ostream& operator<<(std::ostream& os, TokenType type) {
    os << detail::token_type_names[
        to_underlying(type)
    ];
    return os;
}
