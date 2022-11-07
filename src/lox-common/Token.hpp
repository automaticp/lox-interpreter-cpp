#pragma once
#include "TokenType.hpp"
#include "SourceLocation.hpp"
#include "LiteralValue.hpp"
#include <fmt/format.h>
#include <optional>
#include <string>
#include <sstream>
#include <string_view>
#include <utility>
#include <cassert>
#include <cstdint>



class Token {
private:
    std::string lexeme_;        // 32 bytes libstdc++, may change later to boost::container::string (24 bytes)
                                // This might allow the Token to squeeze into 64 bytes total.
                                // Lexemes are usually small and will fit into the short-string buffer.
                                //
                                //
    SourceLocation location_;   // 24 bytes, can be inlined here to save 8 bytes
    bool has_literal_;          // 1 byte (bool, is it guaranteed?)
    TokenType type_;            // 1 byte (uint8_t) + 6 padding :(
                                //
    LiteralValue literal_;      // 32 bytes, stores Nil (aka monostate) when there's no literal

public:
    Token(TokenType type, std::string lexeme, SourceLocation location) :
        type_{ type }, lexeme_{ std::move(lexeme) },
        location_{ std::move(location) }, has_literal_{ false },
        literal_{}
    {}

    Token(TokenType type, std::string lexeme, SourceLocation location, LiteralValue literal) :
        type_{ type }, lexeme_{ std::move(lexeme) },
        location_{ std::move(location) }, has_literal_{ true },
        literal_{ std::move(literal) }
    {}



    TokenType type() const noexcept { return type_; }

    const std::string& lexeme() const noexcept { return lexeme_; }

    bool has_literal() const noexcept { return has_literal_; }
    const LiteralValue& literal() const noexcept { return literal_; }

    uint16_t line() const noexcept { return location_.line; }

    uint16_t column() const noexcept { return location_.column; }

    bool has_file() const noexcept { return location_.has_file(); }
    const std::filesystem::path& file() const noexcept { return *location_.filepath; }

    const SourceLocation& location() const noexcept { return location_; }


    // For debug printing mostly
    std::string info() const {
        return fmt::format(
            "[{:s}] {:s}{:s}",
            to_string(type()),
            lexeme(),
            has_literal() ? " " + to_string(literal()) : ""
        );
    }

    bool operator==(const Token& other) const noexcept {
        bool is_eq{
            type() == other.type() &&
            lexeme() == other.lexeme()
        };
        return is_eq;
    }

    bool operator!=(const Token& other) const noexcept {
        return !(*this == other);
    }

    operator const LiteralValue& () const {
        assert(has_literal());
        return literal();
    }

    operator TokenType() const { return type(); }

};
