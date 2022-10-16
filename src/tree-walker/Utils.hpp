#pragma once
#include <type_traits>
#include <string_view>
#include <array>
#include <charconv>
#include <cassert>


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
