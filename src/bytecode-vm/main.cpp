#include "Chunk.hpp"
#include "Constants.hpp"
#include "Disassembler.hpp"
#include <iostream>

int main(int argc, const char* argv[]) {

    Chunk chunk;
    Constants constants;
    chunk.emit(OP::CONSTANT);
    chunk.emit(constants.emplace_back(6.9));
    chunk.emit(OP::CONSTANT);
    chunk.emit(constants.emplace_back(4.33));
    chunk.emit(OP::RETURN);
    chunk.emit(45);

    Disassembler diss;

    std::cout << diss.disassemble("This Chungus", chunk);

}
