#pragma once
#include "Chunk.hpp"
#include "Constants.hpp"
#include "OpCode.hpp"
#include "ValueStack.hpp"
#include <fmt/core.h>
#include <type_traits>

class VM {
private:
    using ip_t = std::vector<Byte>::const_iterator;

    const Chunk* chunk_;
    ip_t ip_;
    Constants constants_;
    ValueStack stack_;

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
                    fmt::print("Returned: {}\n", stack_.pop());
                    return true;
                case OP::CONSTANT:
                    stack_.push(read_constant());
                    break;
                case OP::NEGATE:
                    stack_.back() = -stack_.back();
                    break;
                case OP::ADD: binary_op(OP::ADD); break;
                case OP::SUBTRACT: binary_op(OP::SUBTRACT); break;
                case OP::MULTIPLY: binary_op(OP::MULTIPLY); break;
                case OP::DIVIDE: binary_op(OP::DIVIDE); break;
                default:
                    return false;
            }
        }
    }

    void binary_op(OP opcode) {
        Value lhs = stack_.pop();
        Value rhs = stack_.pop();
        switch (opcode) {
            case OP::ADD:
                stack_.push(lhs + rhs);
                break;
            case OP::SUBTRACT:
                stack_.push(lhs - rhs);
                break;
            case OP::MULTIPLY:
                stack_.push(lhs * rhs);
                break;
            case OP::DIVIDE:
                stack_.push(lhs / rhs);
                break;
            default:
                assert(false && "Unknown binary op");
                break;
        }
    }


    [[nodiscard]]
    Byte read_byte() noexcept {
        return *ip_++;
    }

    [[nodiscard]]
    const Value& read_constant() noexcept {
        return chunk_->constants()[*ip_++];
    }


};
