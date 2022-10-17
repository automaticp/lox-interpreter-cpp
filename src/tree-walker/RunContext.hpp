#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Scanner.hpp"
#include "Parser.hpp"
#include "Interpreter.hpp"
#include "ExprVisitors.hpp"
#include <string>
#include <optional>
#include <fstream>
#include <iostream>


class RunContext {
private:
    bool is_debug_scanner_;
    bool is_debug_parser_;
    std::optional<std::string> filename_;
    ErrorReporter& err_;
    Interpreter intepreter_;
public:
    RunContext(ErrorReporter& err_reporter,
        bool is_debug_scanner, bool is_debug_parser,
        std::optional<std::string> filename = std::nullopt) :
        err_{ err_reporter },
        intepreter_{ err_ },
        filename_{ filename },
        is_debug_scanner_{ is_debug_scanner }, is_debug_parser_{ is_debug_parser }
    {}

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
        // filename_ is guaranteed to have value from start_running()
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

        Parser parser{ tokens, err_ };

        if (parser.parse_tokens()) {
            if (is_debug_parser_mode()) {
                std::cout << "[Debug @Parser]:\n";
                for (const auto& stmt: parser.peek_result()) {
                    std::cout << stmt->accept(StmtASTPrinterVisitor{}) << '\n';
                }
            }
        } else {
            return;
        }

        intepreter_.interpret(parser.peek_result());
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





