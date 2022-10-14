#include <cstdlib>
#include <iostream>
#include <string>
#include <optional>
#include <cxxopts.hpp>

#include "RunContext.hpp"

int main(int argc, char* argv[]) {

    cxxopts::Options opts{ "lox-twi", "lox tree-walk interpreter" };
    opts.add_options()
        ("h,help", "Show help and exit")
        ("d,debug", "Run in debug. Prints scanned tokens.")
        ("file", "Input file to be parsed", cxxopts::value<std::string>());
    opts.parse_positional("file");
    opts.positional_help("file");

    auto optres = opts.parse(argc, argv);

    if (optres.count("help")) {
        std::cout << opts.help() << '\n';
        return 0;
    }

    StreamErrorReporter err_reporter{ std::cerr };

    RunContext context{
        err_reporter,
        static_cast<bool>(optres.count("debug")),
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
