#pragma once
#include "Chunk.hpp"
#include <fmt/format.h>
#include <string>
#include <utility>
#include <vector>


class Disassembler {
private:
    std::string repr_;
    const Chunk* current_;
    using iter_t = std::vector<Byte>::const_iterator;

public:
    [[nodiscard]]
    std::string disassemble(const std::string& name, const Chunk& chungus) {
        reset(chungus);
        add_chunk_label(name);
        for (auto it{ bytes().begin() }; it != bytes().end(); /*_*/) {
            it = disassemble_instruction(it);
        }
        return std::move(repr_);
    }

private:
    iter_t disassemble_instruction(iter_t it) {
        Byte byte{ *it };
        switch(OP{ byte }) {
            case OP::RETURN:
                add_op_line(it, "RETURN");
                ++it;
                break;
            case OP::CONSTANT: {
                    auto index = *(it + 1);
                    add_op_line(it, fmt::format("CONSTANT {} ({})", index, current_->constants()[index]));
                    ++it;
                    ++it; // Skip constant
                }
                break;
            default:
                add_op_line(it, fmt::format("UNKNOWN[{:d}]", byte));
                ++it;
                break;
        }
        return it;
    }

    void add_chunk_label(const std::string& label) {
        add_line(fmt::format(".{:s}:", label));
    }

    void add_op_line(iter_t it, const std::string& line) {
        add_line(fmt::format("{:04d} {}", offset(it), line));
    }

    void add_line(const std::string& line) {
        repr_ += line + '\n';
    }

    void reset(const Chunk& chunk) {
        repr_.clear();
        current_ = &chunk;
    }

    size_t offset(iter_t it) const noexcept { return it - bytes().begin(); }
    const Chunk& current() const noexcept { return *current_; }
    const std::vector<Byte>& bytes() const noexcept { return current().bytes(); }

};
