#pragma once
#include "ErrorReporter.hpp"
#include "ContextError.hpp"
#include "ErrorSender.hpp"
#include "IError.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "CommonVisitors.hpp"
#include "Resolver.hpp"
#include "Builtins.hpp"
#include "Frontend.hpp"
#include "Importer.hpp"
#include <filesystem>
#include <string>
#include <optional>
#include <fstream>
#include <iostream>
#include <utility>

class RunContext : private ErrorSender<ContextError> {
private:
    std::optional<std::filesystem::path> filename_;

    Frontend frontend_;
    Interpreter interpreter_;
public:
    RunContext(
        ErrorReporter& err_reporter,
        bool debug_scanner, bool debug_parser,
        std::optional<std::filesystem::path> filename = {}
    ) :
        ErrorSender{ err_reporter },
        frontend_{ err_reporter, { debug_scanner, debug_parser } },
        interpreter_{ err_reporter, frontend_.resolver() },
        filename_{ std::move(filename) }
    {
        setup_builtins(
            interpreter_.get_global_environment(),
            frontend_.resolver()
        );
    }

    Frontend& frontend() noexcept { return frontend_; }
    const Frontend& frontend() const noexcept { return frontend_; }

    void start_running() {
        if (is_prompt_mode()) {
            run_prompt();
        } else {
            run_file();
        }
    }

    bool is_debug_scanner_mode() const noexcept {
        return frontend().config().debug_scanner;
    }

    bool is_debug_parser_mode() const noexcept {
        return frontend().config().debug_parser;
    }

    bool is_prompt_mode() const noexcept {
        return !filename_.has_value();
    }

    bool is_file_mode() const noexcept {
        return filename_.has_value();
    }

    void run_prompt() {

        std::string line{};

        std::cout << "Starting lox interpreter prompt...\n";

        if (is_debug_scanner_mode()) { std::cout << "(Scanner debugging enabled)\n"; }
        if (is_debug_parser_mode()) { std::cout << "(Parser debugging enabled)\n"; }

        std::cout << "> ";
        while (std::getline(std::cin, line)) {
            run(line);
            std::cout << "> ";
        }
        std::cout << std::endl;

    }

    void run_file() {
        // filename_ is guaranteed to have value if called from start_running()
        assert(filename_);
        auto text = Importer::read_file(filename_.value());
        if (text) {
            // The data flow here is awkward, tbh
            run(text.value());
        } else {
            send_error(ContextError::Type::unable_to_open_file, filename_.value());
        }
    }


    void run(const std::string& text) {

        auto new_stmts = frontend().pass(text, filename_);

        bool success = interpreter_.interpret(new_stmts);

        if (!success) {
            frontend().importer().undo_last_successful_pass();
        }
    }

};





