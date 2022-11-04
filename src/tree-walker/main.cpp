#include "RunContext.hpp"
#include "CLIArgs.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <tuple>



int main(int argc, char* argv[]) {

    CLIArgsParser arg_parser{
        "lox-twi", "lox tree-walk interpreter"
    };

    auto args = arg_parser.parse(argc, argv);

    if (args.show_help) {
        std::cout << arg_parser.options().help() << '\n';
        return 0;
    }

    StreamErrorReporter err_reporter{ std::cerr };

    RunContext context{
        err_reporter,
        args.debug_scanner,
        args.debug_parser,
        args.filename
    };

    context.start_running();

    if (context.is_file_mode() && err_reporter.had_errors()) {
        return 1;
    }

    return 0;
}
