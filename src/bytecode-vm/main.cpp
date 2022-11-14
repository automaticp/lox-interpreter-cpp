#include "CLIArgs.hpp"
#include "Chunk.hpp"
#include "CodegenVisitor.hpp"
#include "Constants.hpp"
#include "Disassembler.hpp"
#include "ErrorReporter.hpp"
#include "Frontend.hpp"
#include "RunContext.hpp"
#include "VM.hpp"
#include <iostream>

int main(int argc, const char* argv[]) {

    StreamErrorReporter err{ std::cerr };

    CLIArgsParser arg_parser{
        "lox-bvm", "lox bytecode virtual machine",
        err
    };

    auto args = arg_parser.parse(argc, argv);

    if (args.parse_failed) {
        std::cout << arg_parser.options().help() << '\n';
        return 1;
    }

    if (args.show_help) {
        std::cout << arg_parser.options().help() << '\n';
        return 0;
    }


    RunContext context{ err, args };

    context.start_running();

    if (context.is_file_mode() && err.had_errors()) {
        return 1;
    }

    return 0;
}
