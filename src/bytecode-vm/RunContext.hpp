#pragma once
#include "CLIArgs.hpp"
#include "Disassembler.hpp"
#include "ErrorReporter.hpp"
#include "ErrorSender.hpp"
#include "Frontend.hpp"
#include "IError.hpp"
#include "VM.hpp"
#include "CodegenVisitor.hpp"
#include <fmt/core.h>
#include <optional>
#include <filesystem>




class RunContext : private ErrorSender<SimpleError> {
private:
    std::optional<std::filesystem::path> filename_;

    Frontend frontend_;
    VM vm_;

    bool debug_bytecode;

public:
    RunContext(
        ErrorReporter& err,
        const CLIArgs& config
    ) :
        ErrorSender{ err },
        filename_{ config.filename },
        frontend_{ err, { config.debug_scanner, config.debug_parser } },
        vm_{ /* err */ },
        debug_bytecode{ config.debug_bytecode }
    {
        /* setup builtins */
    }


    void start_running() {
        if (is_prompt_mode()) {
            run_prompt();
        } else {
            run_file();
        }
    }

    Frontend& frontend() noexcept { return frontend_; }
    const Frontend& frontend() const noexcept { return frontend_; }

    // Copy-pasted
    bool is_debug_scanner_mode() const noexcept {
        return frontend().config().debug_scanner;
    }

    bool is_debug_parser_mode() const noexcept {
        return frontend().config().debug_parser;
    }

    bool is_debug_bytecode_mode() const noexcept {
        return debug_bytecode;
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

        if (is_debug_scanner_mode()) {
            std::cout << "(Scanner debugging enabled)\n";
        }
        if (is_debug_parser_mode()) {
            std::cout << "(Parser debugging enabled)\n";
        }
        if (is_debug_bytecode_mode()) {
            std::cout << "(Bytecode debugging enabled)\n";
        }

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
            // FIXME: scuffed, use proper context error class
            send_error(
                fmt::format(
                    "[Error @Context]:\nUnable to open file: {}\n",
                    filename_.value().string()
                )
            );
        }
    }


    void run(const std::string& text) {

        auto new_stmts = frontend().pass(text, filename_);

        if (frontend().has_failed()) {
            return;
        }

        Chunk chunk;
        CodegenVisitor codegen{ error_reporter(), chunk };

        for (const auto& stmt : new_stmts) {
            stmt->accept(codegen);
        }
        chunk.emit(OP::RETURN);

        if (is_debug_bytecode_mode()) {
            Disassembler diss;
            std::cout << diss.disassemble("chunk", chunk);
        }

        if (!vm_.interpret(chunk)) {
            // Ehhh, there's no state yet really,
            // But will have to be done later on.
            frontend().importer().undo_last_successful_pass();
        }
    }



};

