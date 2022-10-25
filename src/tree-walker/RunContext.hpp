#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "ExprVisitors.hpp"
#include "Resolver.hpp"
#include "Builtins.hpp"
#include <string>
#include <optional>
#include <fstream>
#include <iostream>
#include <utility>

class RunContext {
private:
    bool is_debug_scanner_;
    bool is_debug_parser_;
    std::optional<std::string> filename_;
    ErrorReporter& err_;
    Parser parser_;
    Resolver resolver_;
    Interpreter interpreter_;
public:
    RunContext(ErrorReporter& err_reporter,
        bool is_debug_scanner, bool is_debug_parser,
        std::optional<std::string> filename = std::nullopt) :
        err_{ err_reporter },
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
        auto text = read_file(filename_.value());
        if (text) {
            run(text.value());
        } else {
            err_.context_error(ContextError::unable_to_open_file, filename_.value());
        }
    }


    void run(const std::string& text) {

        err_.reset();

        Scanner scanner{ text, err_ };

        const auto& tokens = scanner.scan_tokens();

        if (is_debug_scanner_mode()) {
            std::cout << "[Debug @Scanner]:\n";
            for (const auto& token : tokens) {
                std::cout << token.info() << '\n';
            }
        }

        if (err_.had_scanner_errors()) {
            return;
        }


        auto new_stmts = parser_.parse_tokens(tokens);
        if (is_debug_parser_mode()) {
            std::cout << "[Debug @Parser]:\n";
            for (const auto& stmt: new_stmts) {
                std::cout << stmt->accept(StmtASTPrinterVisitor{}) << '\n';
            }
        }

        if (err_.had_parser_errors()) {
            return;
        }

        resolver_.resolve(new_stmts);

        if (err_.had_resolver_errors()) {
            return;
        }

        interpreter_.interpret(new_stmts);
    }

private:
    static std::optional<std::string> read_file(const std::string& filename) {

        std::ifstream fs{ filename };

        if (!fs.fail()) {
            return std::string{
                std::istreambuf_iterator<char>(fs),
                std::istreambuf_iterator<char>()
            };
        } else {
            return std::nullopt;
        }
    }



};





