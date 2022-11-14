#include "Chunk.hpp"
#include "CodegenVisitor.hpp"
#include "Constants.hpp"
#include "Disassembler.hpp"
#include "ErrorReporter.hpp"
#include "Frontend.hpp"
#include "VM.hpp"
#include <iostream>

int main(int argc, const char* argv[]) {


    StreamErrorReporter err{ std::cerr };
    Frontend fe{ err };

    auto new_stmts = fe.pass("print 5 + 6 / -3;");
    if (fe.has_failed()) {
        return 1;
    }

    Chunk chunk;
    CodegenVisitor codegen{ err, chunk };

    for (const auto& stmt : new_stmts) {
        stmt->accept(codegen);
    }
    chunk.emit(OP::RETURN);

    Disassembler diss;

    std::cout << diss.disassemble("this_chungus", chunk);

    VM vm;
    if (!vm.interpret(chunk)) {
        return 1;
    }
    return 0;
}
