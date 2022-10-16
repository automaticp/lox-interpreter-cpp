#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <tuple>
#include <cxxopts.hpp>

#include "RunContext.hpp"

std::tuple<bool, bool> resolve_debug_flags(cxxopts::ParseResult& optres) {
    bool is_scanner_debug{ false };
    bool is_parser_debug{ false };
    if (optres.count("debug")) {
        auto debug_args = optres["debug"].as<std::vector<std::string>>();
        for (const auto& arg : debug_args) {
            if (arg == "scanner") {
                is_scanner_debug = true;
            } else if (arg == "parser") {
                is_parser_debug = true;
            }
        }
    }
    return { is_scanner_debug, is_parser_debug };
}


int main(int argc, char* argv[]) {

    cxxopts::Options opts{ "lox-twi", "lox tree-walk interpreter" };
    opts.add_options()
        ("h,help", "Show help and exit")
        (
            "debug", "Run in debug mode.",
            cxxopts::value<std::vector<std::string>>()->implicit_value("scanner,parser")
        )
        ("file", "Input file to be parsed", cxxopts::value<std::string>());
    opts.parse_positional("file");
    opts.positional_help("file");

    auto optres = opts.parse(argc, argv);

    if (optres.count("help")) {
        std::cout << opts.help() << '\n';
        return 0;
    }

    auto [is_scanner_debug, is_parser_debug] = resolve_debug_flags(optres);

    StreamErrorReporter err_reporter{ std::cerr };

    RunContext context{
        err_reporter,
        is_scanner_debug,
        is_parser_debug,
        std::invoke([&optres]() -> std::optional<std::string> {
            if (optres.count("file")) { return optres["file"].as<std::string>(); }
            else { return std::nullopt; }
        })
    };

    context.start_running();

    if (context.is_file_mode() && err_reporter.had_errors()) {
        return 1;
    }

    return 0;
}
