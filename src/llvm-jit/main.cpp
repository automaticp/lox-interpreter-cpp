#include "ErrorReporter.hpp"
#include "Frontend.hpp"
#include "IRGen.hpp"
#include "IRGenVisitor.hpp"
#include <iostream>


void run(const std::string& text) {
        StreamErrorReporter err{ std::cerr };

        Scanner scanner{ err };

        auto tokens = scanner.scan_tokens(text);

        if (scanner.has_failed()) {
            return;
        }

        Importer importer{ err };

        tokens = importer.resolve_imports(tokens);

        if (importer.has_failed()) {
            return;
        }

        Scanner::append_eof(tokens);

        // No state
        Parser parser{ err };

        auto new_stmts = parser.parse_tokens(tokens);

        if (err.had_errors_of_category(ErrorCategory::parser)) {
            return;
        }

        Resolver resolver{ err };

        resolver.resolve(new_stmts);

        if (err.had_errors_of_category(ErrorCategory::resolver)) {
            return;
        }

        IRGen irgen{ err };

        irgen.generate(new_stmts);

}


int main(int argc, const char* argv[]) {

    run("print 5 + 6;");

}
