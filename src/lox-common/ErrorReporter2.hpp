#pragma once
#include "Token.hpp"
#include "IError.hpp"
#include <boost/unordered_map.hpp>
#include <fmt/core.h>
#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <cassert>
#include <iostream>
#include <algorithm>





class ErrorReporter {
private:
    std::vector<std::unique_ptr<IError>> errors_;

public:
    ErrorReporter() = default;

    void reset() {
        errors_.clear();
    }

    void error(std::unique_ptr<IError> error) {
        assert(error);
        report(*error);
        errors_.emplace_back(std::move(error));
    };

    bool had_errors_of_category(const ErrorCategory& category) const noexcept {
        return std::any_of(
            errors_.begin(), errors_.end(),
            [&category](const auto& err) {
                return err->category() == category;
            }
        );
    }

    bool had_errors() const noexcept {
        return !errors_.empty();
    }

    ErrorReporter(const ErrorReporter&) = delete;
    ErrorReporter& operator=(const ErrorReporter&) = delete;
    ErrorReporter(ErrorReporter&&) = default;
    ErrorReporter& operator=(ErrorReporter&&) = default;
    virtual ~ErrorReporter() = default;
protected:
    virtual void report(const IError& err) = 0;

};



class StreamErrorReporter : public ErrorReporter {
private:
    std::ostream& os_;

public:
    StreamErrorReporter(std::ostream& os) : os_{ os } {}

protected:
    void report(const IError& err) override {
        os_ << err.message();
    }
};










inline std::string_view details_sep(std::string_view details) {
    constexpr size_t line_wrap_limit{ 16 }; // Chosen arbitrarily
    return (details.size() < line_wrap_limit ? ": " : ":\n    ");
}

inline std::string details_tail(std::string_view details) {
    if (details.empty()) return ".";
    return fmt::format(
        "{:s}{:s}.",
        details_sep(details),
        details
    );
}












class ContextError : public IError {
public:
    enum class Type {
        unable_to_open_file
    };

private:
    inline static const boost::unordered_map<Type, std::string_view> messages_{
        {Type::unable_to_open_file, "Unable to open file"},
    };

public:
    Type type;
    std::string details;

    ContextError(Type type, std::string details) :
        type{ type }, details{ std::move(details) }
    {}

    ErrorCategory category() const override {
        return ErrorCategory::context;
    }

    std::string message() const override {
        return fmt::format(
            "[Error @Context]:\n{:s}{:s}\n",
            messages_.at(type),
            details_tail(details)
        );
    }

}; // class ContextError;






class ScannerError : public IError {
public:
    enum class Type {
        unexpected_character,
        unterminated_string_literal,
        unterminated_number_literal
    };

private:
    inline static const boost::unordered_map<Type, std::string_view> messages_{
        {Type::unexpected_character, "Unexpected character"},
        {Type::unterminated_string_literal, "Unterminated string literal"},
        {Type::unterminated_number_literal, "Unterminated number literal"},
    };

public:
    Type type;
    size_t line;
    std::string details;

    ScannerError(Type type, size_t line, std::string details) :
        type{ type }, line{ line }, details{ std::move(details) }
    {}

    ErrorCategory category() const override {
        return ErrorCategory::scanner;
    }

    std::string message() const override {
        return fmt::format(
            "[Error @Scanner] at line {:d}\n{:s}{:s}\n",
            line, messages_.at(type),
            details_tail(details)
        );
    }

}; // class ScannerError;




class ParserError : public IError {
public:
    enum class Type {
        unknown_primary_expression,
        missing_opening_paren,
        missing_closing_paren,
        missing_semicolon,
        expected_identifier,
        invalid_assignment_target,
        missing_opening_brace,
        missing_closing_brace
    };

private:
    inline static const boost::unordered_map<Type, std::string_view> messages_{
        {Type::unknown_primary_expression, "Unknown primary expression"},
        {Type::missing_opening_paren, "Missing opening '('"},
        {Type::missing_closing_paren, "Missing closing ')'"},
        {Type::missing_semicolon, "Missing ';' at the end of statement"},
        {Type::expected_identifier, "Expected identifier"},
        {Type::invalid_assignment_target, "Invalid assignment target"},
        {Type::missing_opening_brace, "Missing opening '{'"},
        {Type::missing_closing_brace, "Missing closing '}'"},
    };

public:
    Type type;
    Token token;
    std::string details;

    ParserError(Type type, Token token, std::string details) :
        type{ type }, token{ std::move(token) },
        details{ std::move(details) }
    {}

    ErrorCategory category() const override {
        return ErrorCategory::parser;
    }

    std::string message() const override {
        return fmt::format(
            "[Error @Parser] at line {:d} token {:s}:\n{:s}{:s}\n",
            token.line, token.lexeme,
            messages_.at(type), details_tail(details)
        );
    }

}; // class ParserError;






class ResolverError : public IError {
public:
    enum class Type {
        initialization_from_self,
        local_variable_redeclaration,
        return_from_global_scope,
        undefined_variable
    };

private:
    inline static const boost::unordered_map<Type, std::string_view> messages_{
        {Type::initialization_from_self, "Initialized name on the right-hand side of the initializer statement is forbidden"},
        {Type::local_variable_redeclaration, "Redeclaration of a local variable"},
        {Type::return_from_global_scope, "Return from global scope"},
        {Type::undefined_variable, "Undefined variable"},
    };

public:
    Type type;
    Token token;
    std::string details;
    std::string expr_or_stmt_name;

    ResolverError(Type type, Token token, std::string expr_or_stmt_name, std::string details) :
        type{ type },
        token{ std::move(token) },
        expr_or_stmt_name{ std::move(expr_or_stmt_name) },
        details{ std::move(details) }
    {}

    ErrorCategory category() const override {
        return ErrorCategory::resolver;
    }

    std::string message() const override {
        return fmt::format(
            "[Error @Resolver] at line {:d} in {:s} ({:s}):\n{:s}{:s}\n",
            token.line, expr_or_stmt_name, token.lexeme,
            messages_.at(type), details_tail(details)
        );
    }

}; // class ResolverError;






// Non-frontend dependency. This is fine. Fix later.
class IExpr;
class ExprGetPrimaryTokenVisitor;
class ExprUserFriendlyNameVisitor;


class InterpreterError : public IError {
public:
    enum class Type {
        unexpected_type,
        undefined_variable,
        wrong_num_of_arguments,
    };

private:
    inline static const boost::unordered_map<Type, std::string_view> messages_{
        {Type::unexpected_type, "Unexpected type"},
        {Type::undefined_variable, "Undefined variable"},
        {Type::wrong_num_of_arguments, "Wrong number of arguments"},
    };

public:
    Type type;
    Token token;
    std::string details;
    std::string expr_name;

    InterpreterError(Type type, const IExpr& expr, std::string details) :
        type{ type },
        token{ expr.accept(ExprGetPrimaryTokenVisitor{}) },
        expr_name{ expr.accept(ExprUserFriendlyNameVisitor{}) },
        details{ std::move(details) }
    {}

    ErrorCategory category() const override {
        return ErrorCategory::interpreter;
    }

    std::string message() const override {
        return fmt::format(
            "[Error @Interpreter] at line {:d} in {:s} ({:s}):\n{:s}{:s}\n",
            token.line, expr_name, token.lexeme,
            messages_.at(type), details_tail(details)
        );
    }

}; // class InterpreterError;


