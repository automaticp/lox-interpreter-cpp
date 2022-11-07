#pragma once
#include "SourceLocation.hpp"
#include <fmt/format.h>
#include <type_traits>
#include <string_view>
#include <string>
#include <array>
#include <charconv>
#include <cassert>
#include <concepts>
#include <variant>


template<typename EnumType>
std::underlying_type_t<EnumType> to_underlying(EnumType value) {
    return static_cast<std::underlying_type_t<EnumType>>(value);
}


inline std::string_view num_to_string(const double& num) {
    thread_local std::array<char, 32> buffer;

    auto [ptr, errc] = std::to_chars(buffer.begin(), buffer.end(), num, std::chars_format::fixed);

    if (errc != std::errc{}) {
        assert(false);
        return "?number_conversion_failed?";
    }

    return { buffer.data(), ptr };
}



// Applies to std::variant with std::visit
template<typename V, typename T>
concept visitor_of =
    requires(V&& visitor, const T& variant) {

    std::visit(std::forward<V>(visitor), variant);
};



template <typename T, typename U>
concept not_same_as_remove_cvref =
    !std::same_as<
        std::remove_cvref_t<T>,
        std::remove_cvref_t<U>
    >;



template <typename T, typename U>
concept not_derived_from_remove_cvref =
    !std::derived_from<
        std::remove_cvref_t<T>,
        std::remove_cvref_t<U>
    >;







namespace detail {

// Utils for error reporting

inline std::string_view details_sep(std::string_view details) {
    constexpr size_t line_wrap_limit{ 16 }; // Chosen arbitrarily
    return (details.size() < line_wrap_limit ? ": " : ":\n    ");
}

inline std::string details_tail(std::string_view details) {
    if (details.empty()) return ".";
    return fmt::format(
        "{:s}{:s}.",
        details_sep(details),
        details
    );
}


inline std::string location_info(const SourceLocation& location) {
    if (location.has_file()) {
        return fmt::format(
            "{:s}:{:d}:{:d}",
            location.file().string(), location.line, location.column
        );
    } else {
        return fmt::format(
            "{:d}:{:d}",
            location.line, location.column
        );
    }
}


} // namespace detail


