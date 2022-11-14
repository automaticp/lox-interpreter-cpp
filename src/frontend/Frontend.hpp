#pragma once
#include "ErrorReporter.hpp"
#include "IError.hpp"
#include "Scanner.hpp"
#include "Importer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <vector>

struct FrontendConfig {
    bool debug_scanner{ false };
    bool debug_parser{ false };
};


class Frontend {
private:
    ErrorReporter& err_;
    FrontendConfig config_;
    Importer importer_;
    Parser parser_;
    Resolver resolver_;

    bool has_failed_{};

public:
    explicit Frontend(ErrorReporter& err,
        FrontendConfig config = { false, false }) :
        err_{ err },
        config_{ config },
        importer_{ err },
        parser_{ err },
        resolver_{ err }
    {}

    Importer& importer() noexcept { return importer_; }
    Parser& parser() noexcept { return parser_; }
    Resolver& resolver() noexcept { return resolver_; }

    FrontendConfig& config() noexcept { return config_; }
    const FrontendConfig& config() const noexcept { return config_; }

    std::span<std::unique_ptr<Stmt>> pass(const std::string& text, std::optional<std::filesystem::path> file = {}) {
        begin_new_pass();

        // FIXME: do not rely on reported errors
        // for control flow and early returns.
        err_.reset();

        Scanner scanner{ err_ };

        std::vector<Token> tokens;
        if (file.has_value()) {
            // FIXME: This is a hack to get the initial import working.
            // Otherwise the 'file' stays relative to the starting dir,
            // but we change directory to the parent of the file.
            // Then the Scanner tries to make a canonical path out of
            // 'file', but fails because it can't find it from the
            // new path. To remedy this, we just make the path
            // absolute/canonical right away.
            file = std::filesystem::canonical(file.value());
            importer().mark_imported(file.value());
            // FIXME: should the Frontend handle this at all?
            std::filesystem::current_path(file->parent_path());
            tokens = scanner.scan_tokens(text, file.value());
        } else {
            tokens = scanner.scan_tokens(text);
        }

        // FIXME (maybe): The frontend pieces could handle this themselves
        if (config_.debug_scanner) {
            std::cout << "[Debug @Scanner]:\n";
            for (const auto& token : tokens) {
                std::cout << token.info() << '\n';
            }
        }


        if (scanner.has_failed()) {
            has_failed_ = true;
            return {};
        }

        tokens = importer().resolve_imports(tokens);

        if (importer_.has_failed()) {
            has_failed_ = true;
            return {};
        }

        Scanner::append_eof(tokens);

        auto new_stmts = parser().parse_tokens(tokens);

        if (config_.debug_parser) {
            std::cout << "[Debug @Parser]:\n";
            for (const auto& stmt: new_stmts) {
                std::cout << stmt->accept(ASTPrintVisitor{}) << '\n';
            }
        }

        // if (parser_.has_failed()) { ... }

        if (err_.had_errors_of_category(ErrorCategory::parser)) {
            importer().undo_last_successful_pass();
            has_failed_ = true;
            return {};
        }

        resolver().resolve(new_stmts);

        if (err_.had_errors_of_category(ErrorCategory::resolver)) {
            importer().undo_last_successful_pass();
            has_failed_ = true;
            return {};
        }

        return new_stmts;
    }


    bool has_failed() const noexcept { return has_failed_; }

private:
    void begin_new_pass() noexcept {
        has_failed_ = false;
    }

};

