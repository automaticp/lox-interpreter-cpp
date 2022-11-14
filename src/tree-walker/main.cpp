#include "RunContext.hpp"
#include "CLIArgs.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <tuple>



int main(int argc, const char* argv[]) {

    std::ios::sync_with_stdio(false);

    StreamErrorReporter err_reporter{ std::cerr };

    CLIArgsParser arg_parser{
        "lox-twi", "lox tree-walk interpreter", err_reporter
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
