#pragma once
#include <cstddef>
#include <string>
#include <array>
#include "Utils.hpp"



enum class ContextError : size_t {

    unable_to_open_file = 0,

};

enum class ScannerError : size_t {

    unexpected_character = 0,
    unterminated_string_literal,
    unterminated_number_literal

};

enum class ParserError : size_t {

    unknown_primary_expression = 0,
    missing_opening_paren,
    missing_closing_paren,
    missing_semicolon,
    expected_identifier,
    invalid_assignment_target,
    missing_opening_brace,
    missing_closing_brace

};


enum class InterpreterError : size_t {

    unexpected_type = 0,
    undefined_variable,
    wrong_num_of_arguments

};


enum class ResolverError : size_t {

    initialization_from_self = 0,
    local_variable_redeclaration,
    return_from_global_scope,
    undefined_variable

};


namespace detail {

inline constexpr std::array context_error_msgs{
    "Unable to open file",
};

inline constexpr std::array scanner_error_msgs{
    "Unexpected character",
    "Unterminated string literal",
    "Unterminated number literal"
};

inline constexpr std::array parser_error_msgs{
    "Unknown primary expression",
    "Missing opening '('",
    "Missing closing ')'",
    "Missing ';' at the end of statement",
    "Expected identifier",
    "Invalid assignment target",
    "Missing opening '{'",
    "Missing closing '}'"
};

inline constexpr std::array interpreter_error_msgs{
    "Unexpected type",
    "Undefined variable",
    "Wrong number of arguments"
};

inline constexpr std::array resolver_error_msgs{
    "Initialized name on the right-hand side of the initializer statement is forbidden",
    "Redeclaration of a local variable",
    "Return from global scope",
    "Undefined variable"
};


} // namepace detail

inline std::string_view to_error_message(ContextError err) {
    return detail::context_error_msgs[ // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        to_underlying(err)
    ];
}

inline std::string_view to_error_message(ScannerError err) {
    return detail::scanner_error_msgs[ // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        to_underlying(err)
    ];
}

inline std::string_view to_error_message(ParserError err) {
    return detail::parser_error_msgs[ // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        to_underlying(err)
    ];
}

inline std::string_view to_error_message(InterpreterError err) {
    return detail::interpreter_error_msgs[ // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        to_underlying(err)
    ];
}

inline std::string_view to_error_message(ResolverError err) {
    return detail::resolver_error_msgs[ // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        to_underlying(err)
    ];
}
