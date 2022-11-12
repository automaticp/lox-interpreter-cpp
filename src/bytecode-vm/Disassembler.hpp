#pragma once
#include "Chunk.hpp"
#include <fmt/format.h>
#include <string>
#include <utility>


class Disassembler {
private:
    std::string repr_;

public:
    [[nodiscard]]
    std::string disassemble(const std::string& name, const Chunk& chungus) {
        clear();
        add_chunk_label(name);
        for (size_t offset{ 0 }; offset < chungus.bytes().size(); /*_*/) {
            offset = disassemble_instruction(chungus.bytes()[offset], offset);
        }
        return std::move(repr_);
    }

private:
    size_t disassemble_instruction(Byte byte, size_t offset) {
        switch(OP{ byte }) {
            case OP::RETURN:
                add_op_line("RETURN", offset);
                ++offset;
                break;
            default:
                add_op_line(fmt::format("UNKNOWN[{:d}]", byte), offset);
                ++offset;
                break;
        }
        return offset;
    }

    void add_chunk_label(const std::string& label) {
        add_line(fmt::format(".{:s}:", label));
    }

    void add_op_line(const std::string& line, size_t offset) {
        add_line(fmt::format("{:04d} {}", offset, line));
    }

    void add_line(const std::string& line) {
        repr_ += line + '\n';
    }

    void clear() {
        repr_.clear();
    }

};
