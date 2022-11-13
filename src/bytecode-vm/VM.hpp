#pragma once
#include "Chunk.hpp"
#include "Constants.hpp"
#include "OpCode.hpp"
#include <fmt/core.h>
#include <type_traits>

class VM {
private:
    using ip_t = std::vector<Byte>::const_iterator;

    const Chunk* chunk_;
    ip_t ip_;
    Constants constants_;

public:
    bool interpret(const Chunk& chunk) {
        chunk_ = &chunk;
        ip_ = chunk.begin();
        return run();
    }


private:
    bool run() {
        while (true) {
            Byte instruction{ read_byte() };
            switch (OP{ instruction }) {
                case OP::RETURN:
                    return true;
                case OP::CONSTANT:
                    fmt::print("{}\n", read_constant());
                    break;
                default:
                    return false;
            }
        }
    }


    Byte read_byte() noexcept {
        return *ip_++;
    }

    const Value& read_constant() noexcept {
        return chunk_->constants()[*ip_++];
    }


};
