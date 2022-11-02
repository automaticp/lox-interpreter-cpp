#pragma once
#include "IError.hpp"
#include "Utils.hpp"
#include <boost/unordered_map.hpp>
#include <fmt/format.h>


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
            detail::details_tail(details)
        );
    }

}; // class ContextError;


