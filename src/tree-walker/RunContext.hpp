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
            importer_.mark_imported(filename_.value());
            run(text.value());
        } else {
            send_error(ContextError::Type::unable_to_open_file, filename_.value());
        }
    }


    void run(const std::string& text) {

        error_reporter().reset();

        Scanner scanner{ error_reporter() };

        auto tokens = scanner.scan_tokens(text);

        if (is_debug_scanner_mode()) {
            std::cout << "[Debug @Scanner]:\n";
            for (const auto& token : tokens) {
                std::cout << token.info() << '\n';
            }
        }

        // This error checking is eww, tbh
        if (error_reporter().had_errors_of_category(ErrorCategory::scanner)) {
            return;
        }

        tokens = importer_.resolve_imports(tokens);

        if (error_reporter().had_errors_of_category(ErrorCategory::import)) {
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

        if (error_reporter().had_errors_of_category(ErrorCategory::parser)) {
            return;
        }

        resolver_.resolve(new_stmts);

        if (error_reporter().had_errors_of_category(ErrorCategory::resolver)) {
            return;
        }

        interpreter_.interpret(new_stmts);
    }

};





