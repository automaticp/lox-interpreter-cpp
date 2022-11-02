#pragma once
#include "IError.hpp"
#include "Token.hpp"
#include "Utils.hpp"
#include "CommonVisitors.hpp"
#include "Expr.hpp"
#include <boost/unordered_map.hpp>
#include <fmt/format.h>


// Non-frontend dependency. This is fine. Fix later.


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

    InterpreterError(Type type, const Expr& expr, std::string details) :
        type{ type },
        token{ expr.accept(ExprGetPrimaryTokenVisitor{}) },
        expr_name{ expr.accept(UserFriendlyNameVisitor{}) },
        details{ std::move(details) }
    {}

    ErrorCategory category() const override {
        return ErrorCategory::interpreter;
    }

    std::string message() const override {
        return fmt::format(
            "[Error @Interpreter] at line {:d} in {:s} ({:s}):\n{:s}{:s}\n",
            token.line, expr_name, token.lexeme,
            messages_.at(type), detail::details_tail(details)
        );
    }

}; // class InterpreterError;


