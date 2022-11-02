#pragma once
#include "IError.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include <boost/unordered_map.hpp>
#include <fmt/format.h>
#include <string>
#include <string_view>
#include <utility>




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
            detail::details_tail(details)
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
            messages_.at(type), detail::details_tail(details)
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
            messages_.at(type), detail::details_tail(details)
        );
    }

}; // class ResolverError;
