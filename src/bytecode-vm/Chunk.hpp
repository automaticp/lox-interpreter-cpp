#pragma once
#include "Utils.hpp"
#include "OpCode.hpp"
#include <vector>


class Chunk {
private:
    std::vector<Byte> bytes_;

public:
    void emit(OP opcode) {
        bytes_.emplace_back(to_underlying(opcode));
    }

    void emit(Byte byte) {
        bytes_.emplace_back(byte);
    }

    const std::vector<Byte>& bytes() const noexcept { return bytes_; }
};


