#include "Scanner.hpp"
#include "ErrorReporter.hpp"
#include <doctest/doctest.h>
#include <iostream>

TEST_SUITE("Scanner") {

TEST_CASE("init-expressions") {

    StreamErrorReporter err{ std::cerr };

    SUBCASE("number") {
        Scanner s{ R"(var num = 56;)", err };

        const auto& tokens = s.scan_tokens();

        REQUIRE(tokens.size() == 5);

        CHECK(tokens[0] == Token{ TokenType::kw_var, "var", 1 });
        CHECK(tokens[1] == Token{ TokenType::identifier, "num", 1 });
        CHECK(tokens[2] == Token{ TokenType::eq, "=", 1 });
        CHECK(tokens[3] == Token{ TokenType::number, "56", 1 });
        CHECK(tokens[4] == Token{ TokenType::semicolon, ";", 1 });
    }

    SUBCASE("string") {
        Scanner s{ R"(var str = "Hello, World!";)", err };

        const auto& tokens = s.scan_tokens();

        REQUIRE(tokens.size() == 5);

        CHECK(tokens[0] == Token{ TokenType::kw_var, "var", 1 });
        CHECK(tokens[1] == Token{ TokenType::identifier, "str", 1 });
        CHECK(tokens[2] == Token{ TokenType::eq, "=", 1 });
        CHECK(tokens[3] == Token{ TokenType::string, R"("Hello, World!")", 1 });
        CHECK(tokens[4] == Token{ TokenType::semicolon, ";", 1 });
    }

}

}
