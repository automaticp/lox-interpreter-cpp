#pragma once
#include "ErrorReporter.hpp"
#include "Errors.hpp"
#include "Scanner.hpp"
#include <string>
#include <optional>
#include <fstream>
#include <iostream>


class RunContext {
private:
    bool is_debug_;
    std::optional<std::string> filename_;
    ErrorReporter& err_;
public:
    RunContext(ErrorReporter& err_reporter, bool is_debug, std::optional<std::string> filename) :
        err_{ err_reporter }, filename_{ filename }, is_debug_{ is_debug } {}

    void start_running() {
        if (is_prompt_mode()) {
            run_prompt();
        } else {
            run_file();
        }
    }

    bool is_debug_mode() const noexcept { return is_debug_; }
    bool is_prompt_mode() const noexcept { return !filename_.has_value(); }
    bool is_file_mode() const noexcept { return filename_.has_value(); }

    void run_prompt() {

        std::string line{};

        std::cout << "Starting lox interpreter prompt...\n";

        if (is_debug_mode()) { std::cout << "(Running in debug mode)\n"; }

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
        Scanner scanner{ text, err_ };

        const auto& tokens = scanner.scan_tokens();

        if (is_debug_mode()) {
            for (const auto& token : tokens) {
                std::cout << token.info() << '\n';
            }
        }
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





