#include "Scanner.hpp"
#include <doctest/doctest.h>
#include <iostream>

TEST_SUITE("Scanner") {

TEST_CASE("init-expressions") {

    StreamErrorReporter err{ std::cerr };

    SUBCASE("number") {
        Scanner s{ err };

        auto tokens = s.scan_tokens(R"(var num = 56;)");
        Scanner::append_eof(tokens);

        REQUIRE(tokens.size() == 6);

        CHECK(tokens[0] == Token{ TokenType::kw_var, "var", {1, 3} });
        CHECK(tokens[1] == Token{ TokenType::identifier, "num", {1, 7 } });
        CHECK(tokens[2] == Token{ TokenType::eq, "=", {1, 9} });
        CHECK(tokens[3] == Token{ TokenType::number, "56", {1, 11} });
        CHECK(tokens[4] == Token{ TokenType::semicolon, ";", {1, 12} });
        CHECK(tokens[5] == Token{ TokenType::eof, "?eof?", {1, 13} });
    }

    SUBCASE("string") {
        Scanner s{ err };

        auto tokens = s.scan_tokens(R"(var str = "Hello, World!";)");
        Scanner::append_eof(tokens);

        REQUIRE(tokens.size() == 6);

        CHECK(tokens[0] == Token{ TokenType::kw_var, "var", {1, 1} });
        CHECK(tokens[1] == Token{ TokenType::identifier, "str", {1, 1} });
        CHECK(tokens[2] == Token{ TokenType::eq, "=", {1, 1} });
        CHECK(tokens[3] == Token{ TokenType::string, R"("Hello, World!")", {1, 1} });
        CHECK(tokens[4] == Token{ TokenType::semicolon, ";", {1, 1} });
        CHECK(tokens[5] == Token{ TokenType::eof, "?eof?", {1, 1} });
    }

}

TEST_CASE("multiline") {

    StreamErrorReporter err{ std::cerr };

    SUBCASE("if-statement-with-comments") {

        Scanner s{ err };

        auto tokens = s.scan_tokens(
            "// super algorithm\n"
            "if (a > 5) {\n"
            "  /* reduce /***/ */\n"
            "  a = a / 5;\n"
            "}"
        );
        Scanner::append_eof(tokens);

        REQUIRE(tokens.size() == 15);

        CHECK(tokens[0] == Token{ TokenType::kw_if, "if", {2, 1} });
        CHECK(tokens[1] == Token{ TokenType::lparen, "(", {2, 1} });
        CHECK(tokens[2] == Token{ TokenType::identifier, "a", {2, 1} });
        CHECK(tokens[3] == Token{ TokenType::greater, ">", {2, 1} });
        CHECK(tokens[4] == Token{ TokenType::number, "5", {2, 1} });
        CHECK(tokens[5] == Token{ TokenType::rparen, ")", {2, 1} });
        CHECK(tokens[6] == Token{ TokenType::lbrace, "{", {2, 1} });
        CHECK(tokens[7] == Token{ TokenType::identifier, "a", {4, 1} });
        CHECK(tokens[8] == Token{ TokenType::eq, "=", {4, 1} });
        CHECK(tokens[9] == Token{ TokenType::identifier, "a", {4, 1} });
        CHECK(tokens[10] == Token{ TokenType::slash, "/", {4, 1} });
        CHECK(tokens[11] == Token{ TokenType::number, "5", {4, 1} });
        CHECK(tokens[12] == Token{ TokenType::semicolon, ";", {4, 1} });
        CHECK(tokens[13] == Token{ TokenType::rbrace, "}", {5, 1} });
        CHECK(tokens[14] == Token{ TokenType::eof, "?eof?", {5, 1} });
    }



}



}
