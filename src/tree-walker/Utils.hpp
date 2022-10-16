#pragma once
#include <type_traits>


template<typename EnumType>
std::underlying_type_t<EnumType> to_underlying(EnumType value) {
    return static_cast<std::underlying_type_t<EnumType>>(value);
}


