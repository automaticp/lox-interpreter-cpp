#include "Chunk.hpp"
#include "Constants.hpp"
#include "Disassembler.hpp"
#include "VM.hpp"
#include <iostream>

int main(int argc, const char* argv[]) {

    Chunk chunk;
    chunk.emit_constant(6.9);
    chunk.emit_constant(4.33);
    chunk.emit(OP::MULTIPLY);
    chunk.emit(OP::NEGATE);
    chunk.emit_constant(1.0);
    chunk.emit(OP::DIVIDE);
    // chunk.emit(45);
    chunk.emit(OP::RETURN);
    // chunk.emit(46);

    Disassembler diss;

    std::cout << diss.disassemble("this_chungus", chunk);

    VM vm;
    if (!vm.interpret(chunk)) {
        return 1;
    }
    return 0;
}
