#pragma once
#include "ErrorReporter.hpp"
#include "IError.hpp"
#include "ErrorSender.hpp"
#include <cxxopts.hpp>
#include <fmt/format.h>
#include <optional>
#include <string>
#include <utility>


// General Command Line Interface (CLI) parsing utilities.
// Not mandatory, but could be reused for different backends.

struct CLIArgs {
    cxxopts::ParseResult result{};
    std::optional<std::string> filename{};
    bool parse_failed{};
    bool show_help{};
    bool debug_scanner{};
    bool debug_parser{};
    bool debug_bytecode{};
};

class CLIArgsError : public IError {
private:
    std::string message_;

public:
    CLIArgsError(std::string message) : message_{ std::move(message) } {}

    ErrorCategory category() const override {
        return ErrorCategory::context; // FIXME: it's own category maybe?
    }

    std::string message() const override {
        return fmt::format("[Error @Context]:\n{:s}\n", message_);
    }
};


class CLIArgsParser : private ErrorSender<CLIArgsError> {
private:
    cxxopts::Options opts_;

public:
    CLIArgsParser(std::string program, std::string help_string, ErrorReporter& err) :
        opts_{ std::move(program), std::move(help_string) },
        ErrorSender{ err }
    {
        opts_.add_options()
        ("h,help", "Show help and exit")
        (
            "debug", "Run in debug mode.",
            cxxopts::value<std::vector<std::string>>()->implicit_value("scanner,parser,bytecode")
        )
        ("file", "Input file to be parsed", cxxopts::value<std::string>());

        opts_.parse_positional("file");
        opts_.positional_help("file");
    }


    CLIArgs parse(int argc, const char* argv[]) { // NOLINT
        CLIArgs args{};

        try {
            args.result = opts_.parse(argc, argv);
        } catch (cxxopts::OptionParseException& e) {
            send_error(e.what());
            args.parse_failed = true;
            return args;
        }

        if (!resolve_debug_flags(args)) {
            args.parse_failed = true;
            return args;
        }

        args.show_help = args.result.count("help");

        args.filename =
            std::invoke(
                [&args]() -> std::optional<std::filesystem::path> {

                    if (args.result.count("file")) {
                        return args.result["file"].as<std::string>();
                    } else {
                        return {};
                    }
                }
            );

        return args;
    }

    cxxopts::Options& options() noexcept { return opts_; }

private:
    bool resolve_debug_flags(CLIArgs& args) {
        bool failed{ false };

        if (args.result.count("debug")) {

            decltype(auto) debug_args = args.result["debug"].as<std::vector<std::string>>();

            for (const auto& arg : debug_args) {
                if (arg == "scanner") {
                    args.debug_scanner = true;
                } else if (arg == "parser") {
                    args.debug_parser = true;
                } else if (arg == "bytecode") {
                    args.debug_bytecode = true;
                } else {
                    send_error(
                        fmt::format("Unknown debug option: '{:s}'", arg)
                    );
                    // Don't immediately return, report all errors first.
                    failed = true;
                }
            }
        }
        return !failed;
    }

};
