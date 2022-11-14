#pragma once
#include "Constants.hpp"
#include "Utils.hpp"
#include "OpCode.hpp"
#include <vector>
#include <utility>


class Chunk {
private:
    Constants constants_;
    std::vector<Byte> bytes_;

public:
    void emit(OP opcode) {
        bytes_.emplace_back(to_underlying(opcode));
    }

    void emit(Byte byte) {
        bytes_.emplace_back(byte);
    }

    void emit_constant(Value val) {
        emit(OP::CONSTANT);
        emit(constants_.emplace_back(std::move(val)));
    }

    const std::vector<Byte>& bytes() const noexcept { return bytes_; }
    const Constants& constants() const noexcept { return constants_; }

    auto begin() noexcept { return bytes_.begin(); }
    auto begin() const noexcept { return bytes_.begin(); }
    auto end() noexcept { return bytes_.end(); }
    auto end() const noexcept { return bytes_.end(); }
};


