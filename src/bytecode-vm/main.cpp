#include "Chunk.hpp"
#include "Disassembler.hpp"
#include <iostream>

int main(int argc, const char* argv[]) {

    Chunk chunk;
    chunk.emit(OP::RETURN);
    chunk.emit(OP{ 12 });

    Disassembler diss;

    std::cout << diss.disassemble("This Chungus", chunk);

}
