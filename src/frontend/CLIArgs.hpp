#pragma once
#include <cxxopts.hpp>
#include <optional>
#include <string>
#include <utility>


// General Command Line Interface (CLI) parsing utilities.
// Not mandatory, but could be reused for different backends.

struct CLIArgs {
    cxxopts::ParseResult result;
    std::optional<std::string> filename;
    bool show_help;
    bool debug_scanner;
    bool debug_parser;
};


class CLIArgsParser {
private:
    cxxopts::Options opts_;

public:
    CLIArgsParser(std::string program, std::string help_string) :
        opts_{ std::move(program), std::move(help_string) }
    {
        opts_.add_options()
        ("h,help", "Show help and exit")
        (
            "debug", "Run in debug mode.",
            cxxopts::value<std::vector<std::string>>()->implicit_value("scanner,parser")
        )
        ("file", "Input file to be parsed", cxxopts::value<std::string>());
        opts_.parse_positional("file");
        opts_.positional_help("file");
    }


    CLIArgs parse(int argc, char* argv[]) { // NOLINT
        auto result = opts_.parse(argc, argv);

        auto [debug_scanner, debug_parser] =
            resolve_debug_flags(result);
        bool show_help = result.count("help");

        // FIXME: Maybe report a ContextError when the
        // arg parser fails?

        auto filename =
            std::invoke(
                [&result]() -> std::optional<std::string> {

                    if (result.count("file")) {
                        return result["file"].as<std::string>();
                    } else {
                        return {};
                    }
                }
            );

        return {
            // NOLINTNEXTLINE: ParseResult is move-assignable, but not move-constructable. An oversight, perhaps?
            .result=std::move(result),
            .filename=std::move(filename),
            .show_help=show_help,
            .debug_scanner=debug_scanner,
            .debug_parser=debug_parser
        };
    }

    cxxopts::Options& options() noexcept { return opts_; }

private:
    std::tuple<bool, bool> resolve_debug_flags(cxxopts::ParseResult& optres) {
        bool is_scanner_debug{ false };
        bool is_parser_debug{ false };
        if (optres.count("debug")) {
            auto debug_args =
                optres["debug"].as<std::vector<std::string>>();
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

};
