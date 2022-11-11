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
    bool is_debug_scanner_;
    bool is_debug_parser_;
    std::optional<std::string> filename_;
    Importer importer_;
    Parser parser_;
    Resolver resolver_;
    Interpreter interpreter_;
public:
    RunContext(ErrorReporter& err_reporter,
        bool is_debug_scanner, bool is_debug_parser,
        std::optional<std::string> filename = std::nullopt) :
        ErrorSender{ err_reporter },
        importer_{ err_reporter },
        parser_{ err_reporter },
        resolver_{ err_reporter },
        interpreter_{ err_reporter, resolver_ },
        filename_{ std::move(filename) },
        is_debug_scanner_{ is_debug_scanner }, is_debug_parser_{ is_debug_parser }
    {
        setup_builtins(interpreter_.get_global_environment(),  resolver_);
    }

    void start_running() {
        if (is_prompt_mode()) {
            run_prompt();
        } else {
            run_file();
        }
    }

    bool is_debug_scanner_mode() const noexcept { return is_debug_scanner_; }
    bool is_debug_parser_mode() const noexcept { return is_debug_parser_; }
    bool is_prompt_mode() const noexcept { return !filename_.has_value(); }
    bool is_file_mode() const noexcept { return filename_.has_value(); }

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
            std::filesystem::path file{
                std::filesystem::canonical(filename_.value())
            };

            // FIXME: This is a hack to get the initial import working.
            // Otherwise the filename_ stays relative to the starting dir,
            // but we change directory to the parent of the file.
            // Then the Scanner tries to make a canonical path out of
            // filename_, but fails because it can't find the file.
            // To remedy this, we just make the path absolute/canonical
            // right away.
            filename_ = file.string();

            importer_.mark_imported(file);

            // cd into the top-level file dir
            std::filesystem::current_path(file.parent_path());

            run(text.value());
        } else {
            send_error(ContextError::Type::unable_to_open_file, filename_.value());
        }
    }


    void run(const std::string& text) {

        error_reporter().reset();

        Scanner scanner{ error_reporter() };

        auto tokens = is_file_mode() ?
            scanner.scan_tokens(text, filename_.value()) :
            scanner.scan_tokens(text);

        if (is_debug_scanner_mode()) {
            std::cout << "[Debug @Scanner]:\n";
            for (const auto& token : tokens) {
                std::cout << token.info() << '\n';
            }
        }

        if (scanner.has_failed()) {
            return;
        }

        tokens = importer_.resolve_imports(tokens);

        if (importer_.has_failed()) {
            return;
        }

        Scanner::append_eof(tokens);

        auto new_stmts = parser_.parse_tokens(tokens);
        if (is_debug_parser_mode()) {
            std::cout << "[Debug @Parser]:\n";
            for (const auto& stmt: new_stmts) {
                std::cout << stmt->accept(ASTPrintVisitor{}) << '\n';
            }
        }

        // This error checking is eww, tbh
        if (error_reporter().had_errors_of_category(ErrorCategory::parser)) {
            importer_.undo_last_successful_pass();
            return;
        }

        resolver_.resolve(new_stmts);

        if (error_reporter().had_errors_of_category(ErrorCategory::resolver)) {
            importer_.undo_last_successful_pass();
            return;
        }

        bool success = interpreter_.interpret(new_stmts);

        if (!success) {
            importer_.undo_last_successful_pass();
        }
    }

};





