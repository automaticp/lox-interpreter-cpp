#pragma once
#include <cstddef>
#include <string>
#include <array>
#include <type_traits>

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
    missing_closing_paren

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
    "Missing closing ')'"
};


} // namepace detail

inline std::string_view to_error_message(ContextError err) {
    return detail::context_error_msgs[
        static_cast<std::underlying_type_t<ContextError>>(err)
    ];
}

inline std::string_view to_error_message(ScannerError err) {
    return detail::scanner_error_msgs[
        static_cast<std::underlying_type_t<ScannerError>>(err)
    ];
}

inline std::string_view to_error_message(ParserError err) {
    return detail::parser_error_msgs[
        static_cast<std::underlying_type_t<ParserError>>(err)
    ];
}
