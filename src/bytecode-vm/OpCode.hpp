#pragma once
#include <cstdint>
#include <cstddef>

using Byte = unsigned char;

// Capital letters make it *feel* low-level
enum class OP : Byte {
    RETURN,
};

