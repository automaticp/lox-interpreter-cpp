#pragma once
#include <type_traits>
#include <string_view>
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
    requires(const V& visitor, const T& variant) {

    std::visit(V{}, variant);
    std::visit(visitor, variant);
};



template <typename T, typename U>
concept not_same_as_remove_cvref =
    !std::same_as<
        std::remove_cvref_t<T>,
        std::remove_cvref_t<U>
    >;



