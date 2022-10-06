#pragma once

enum class TokenType {

    // Single-character
    lparen, rparen, lbrace, rbrace,
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
