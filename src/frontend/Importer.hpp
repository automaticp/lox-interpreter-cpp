#pragma once
#include "ErrorReporter.hpp"
#include "TokenIterator.hpp"
#include "ErrorSender.hpp"
#include "TokenType.hpp"
#include "Token.hpp"
#include "FrontendErrors.hpp"
#include "Scanner.hpp"
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <ios>
#include <iterator>
#include <vector>
#include <utility>



class Importer : private ErrorSender<ImporterError> {
private:
    class ImportResolver : private ErrorSender<ImporterError> {
    private:
        TokenIterator<std::vector<Token>::const_iterator> state_;
        Importer& importer_; // To update and check the list of already imported files

    public:
        ImportResolver(ErrorReporter& err, Importer& importer) :
            ErrorSender{ err }, importer_{ importer } {}


        // Try to recursively import starting from the file at path.
        // On failure, reports to the ErrorReporter and throws ImporterError::Type.
        [[nodiscard]]
        std::vector<Token> try_resolve_imports(std::vector<Token> tokens) {
            // FIXME: maybe don't copy the tokens but just mutate by ref?

            // Comments here mirror the steps that I've written down
            // in my notebook when trying to wrap my head around the
            // order of operations. Sorry if they seem too redundant.


            // 1 Initialize the TokenIterator state
            state_.reset(tokens.begin(), tokens.end());

            // 2. Search for import statements (loop)
            while (!state_.is_end()) {
                if (state_.match(TokenType::kw_import)) {
                    // 2.5 Parse the import statement and get the file path
                    auto new_file = import_stmt();

                    // 2.75 Check that the imported file hasn't yet been imported.
                    // Skip it if it was.
                    if (!importer_.is_already_imported(new_file)) {

                        // 3. Create an ImportResolver for a new file
                        ImportResolver impres{ error_reporter(), importer_ };

                        // 4. Read and scan the file.
                        // Call from this, not from impres, in order to report a correct token.
                        auto source = try_read(new_file);

                        Scanner scanner{ error_reporter() };

                        // These are 'flat' tokens, with imports unresolved.
                        auto new_file_tokens =
                            scanner.scan_tokens(source, new_file);

                        // Check if the scanner succeded
                        if (scanner.has_failed()) {
                            // Abort directly, don't send the error,
                            // as it must've already been reported by the Scanner.
                            // FIXME: Maybe make it's own error type?
                            abort_by_exception(ImporterError::Type::failed_to_read_file);
                        }

                        // 3.75 Mark this file as imported
                        importer_.mark_imported_this_pass(new_file);

                        // 4. Resolve imports for the new file
                        // (recursively calls this function)
                        auto resolved_tokens =
                            impres.try_resolve_imports(std::move(new_file_tokens));

                        // 5. Move-insert the result into the token vector
                        auto it = tokens.insert(
                            state_.current(), // Points at the token after the import statement
                            std::make_move_iterator(resolved_tokens.begin()),
                            std::make_move_iterator(resolved_tokens.end())
                        );

                        // 6. Update iterators (state):
                        // State is updated after each insertion
                        // to account for iterator invalidation.
                        state_.reset(
                            tokens.begin(),
                            // New tokens already have their imports resolved
                            // due to depth-first recursion, skip them.
                            it + std::ssize(resolved_tokens),
                            tokens.end()
                        );

                    }

                    // 7. Repeat search until there's no more import statements (loop)
                } else {
                    state_.advance();
                }
            }

            return tokens;
        }




    private:
        // Parse the import statement and return the import path
        std::filesystem::path import_stmt() {
            const Token& path_tok = try_consume(
                TokenType::string, ImporterError::Type::expected_import_string
            );

            assert(path_tok.has_literal());
            const auto& path_ref = std::get<String>(path_tok.literal());
            // Range init cause String is from boost.
            std::filesystem::path filepath{ path_ref.begin(), path_ref.end() };

            try_consume(
                TokenType::semicolon, ImporterError::Type::missing_semicolon
            );

            return filepath;
        }



        // Try reading the file at path, abort on failure
        std::string try_read(const std::filesystem::path& path) {

            // Must've skipped past the end of statement:
            // import "file.lox";
            //                   ^--current()
            // So we go back 2 tokens to get the name of the file.
            const Token& path_tok = *(state_.current() - 2);

            if (!std::filesystem::exists(path)) {
                report_error_and_abort(
                    ImporterError::Type::path_does_not_exist, path_tok, path.string()
                );
            }

            if (!std::filesystem::is_regular_file(path)) {
                report_error_and_abort(
                    ImporterError::Type::not_a_regular_file, path_tok, path.string()
                );
            }

            auto text = read_file(path);

            if (!text.has_value()) {
                report_error_and_abort(
                    ImporterError::Type::failed_to_read_file, path_tok, path.string()
                );
            }

            return std::move(text.value()); // NOLINT: optional checked above, aborts if empty
        }





        // Ctrl+C, Ctrl+V from Parser.
        const Token& try_consume(TokenType expected, ImporterError::Type fail_error) {
            if (!state_.match(expected)) {
                report_error_and_abort(fail_error);
            }
            return state_.peek_previous();
        }

        void abort_by_exception(ImporterError::Type type) const noexcept(false) {
            throw type;
        }

        void report_error(ImporterError::Type type, std::string_view details = "") {
            send_error(type, state_.peek().location(), std::string(details));
        }

        void report_error(ImporterError::Type type, const Token& token, std::string_view details = "") {
            send_error(type, token.location(), std::string(details));
        }


        void report_error_and_abort(ImporterError::Type type, std::string_view details = "") {
            report_error(type, details);
            abort_by_exception(type);
        }

        void report_error_and_abort(ImporterError::Type type, const Token& token, std::string_view details = "") {
            report_error(type, token, details);
            abort_by_exception(type);
        }



    }; // class ImportResolver


    // A list of all succesfully imported files.
    // Updated each time the call to resolve_imports() succeeds.
    std::vector<std::filesystem::path> imported_files_;

    // A list of imported files during this call to resolve_imports().
    // Reset on each invokation of resolve_imports().
    std::vector<std::filesystem::path> imported_this_pass_;

    // A flag indicating whether the last call to resolve_imports() has failed.
    // Reset on each invokation of resolve_imports().
    bool has_failed_{};

    // An iterator pointing to the beginning of the segment,
    // inserted into the imported_files_ on the last pass.
    std::vector<std::filesystem::path>::const_iterator last_insertion_point_;

public:
    explicit Importer(ErrorReporter& err) : ErrorSender{ err } {}


    [[nodiscard("Successful import marks imported files as not reimportable.")]]
    std::vector<Token> resolve_imports(const std::vector<Token>& tokens) {
        begin_new_import_pass();
        ImportResolver impres{ error_reporter(), *this };
        try {
            auto new_tokens = impres.try_resolve_imports(tokens);
            append_imported_this_pass_on_success();
            return new_tokens;
        } catch (ImporterError::Type) {
            has_failed_ = true;
            return {};
        }
    }

    // Used by frontend to manually mark the top-level file as imported.
    void mark_imported(std::filesystem::path filepath) {
        imported_files_.emplace_back(std::move(filepath));
    }

    // Validate that the last call to resolve_imports() succeded.
    bool has_failed() const noexcept { return has_failed_; }

    // If the import pass succeeds, but later passes (Parser, Resolver, etc.) fail,
    // this provides the mechanism to rollback the list of imported files, so that
    // these files would be reimportable.
    //
    // Repeated calls will not erase any more elements.
    void undo_last_successful_pass() {
        last_insertion_point_ = imported_files_.erase(last_insertion_point_, imported_files_.cend());
    }


    static std::optional<std::string> read_file(const std::filesystem::path& file) {

        std::ifstream fs{ file };

        if (!fs.fail()) {
            try {
                return std::string{
                    std::istreambuf_iterator<char>(fs),
                    std::istreambuf_iterator<char>()
                };
            } catch (std::ios_base::failure& e) {
                // return nullopt;
            }
        }
        return {};
    }




private:
    // Checked by ImportResolver to prevent repeating or circular imports.
    bool is_already_imported(const std::filesystem::path& file) const {

        auto is_same_file = [&file](const std::filesystem::path& other) {
            return std::filesystem::equivalent(file, other);
        };

        return std::any_of(
            imported_files_.begin(),
            imported_files_.end(),
            is_same_file
        ) || std::any_of(
            imported_this_pass_.begin(),
            imported_this_pass_.end(),
            is_same_file
        );
    }

    // Marked by ImportResolver upon reading a file.
    void mark_imported_this_pass(const std::filesystem::path& file) {
        imported_this_pass_.emplace_back(file);
    }

    // Called on each invokation of resolve_imports().
    // Resets the per-call state.
    void begin_new_import_pass() {
        imported_this_pass_.clear();
        has_failed_ = false;
    }


    // If the top level call to try_resolve_imports() succeeds, then append the files
    // imported during the call to the list of all imported files.
    void append_imported_this_pass_on_success() {
        last_insertion_point_ = imported_files_.insert(
            imported_files_.end(),
            std::make_move_iterator(imported_this_pass_.begin()),
            std::make_move_iterator(imported_this_pass_.end())
        );
    }

};


