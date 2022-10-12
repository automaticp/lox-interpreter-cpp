#include <doctest/doctest.h>
#include <memory>
#include "ExprVisitors.hpp"
#include "Expr.hpp"
#include "TokenType.hpp"


static auto print(const IExpr& expr) {
    return expr.accept(ExprASTPrinterVisitor{});
}


TEST_SUITE("ASTPrinter") {

TEST_CASE("literals") {

    LiteralExpr l_string{ "hello" };
    LiteralExpr l_number{ 5.6 };
    LiteralExpr l_boolean{ true };
    LiteralExpr l_nil{ nullptr };

    CHECK(print(l_string)   == "\"hello\"");
    CHECK(print(l_number)   == "5.600000");
    CHECK(print(l_boolean)  == "true");
    CHECK(print(l_nil)      == "nil");

}


TEST_CASE("unary") {

    UnaryExpr u_not_true{ TokenType::bang, std::make_unique<LiteralExpr>(true) };
    UnaryExpr u_minus_num{ TokenType::minus, std::make_unique<LiteralExpr>(5.6) };

    CHECK(print(u_not_true) == "(! true)");
    CHECK(print(u_minus_num) == "(- 5.600000)");

}


TEST_CASE("binary") {

    BinaryExpr b_num_times_num{
        TokenType::star,
        std::make_unique<LiteralExpr>(5.6),
        std::make_unique<LiteralExpr>(0.1)
    };

    BinaryExpr b_false_and_true{
        TokenType::kw_and,
        std::make_unique<LiteralExpr>(false),
        std::make_unique<LiteralExpr>(true)
    };

    BinaryExpr b_hello_plus_world{
        TokenType::plus,
        std::make_unique<LiteralExpr>("hello"),
        std::make_unique<LiteralExpr>("world")
    };

    CHECK(print(b_num_times_num) == "(* 5.600000 0.100000)");
    CHECK(print(b_false_and_true) == "(and false true)");
    CHECK(print(b_hello_plus_world) == "(+ \"hello\" \"world\")");

}


TEST_CASE("group") {

    GroupedExpr g_nil{ std::make_unique<LiteralExpr>(nullptr) };

    CHECK(print(g_nil) == "(group nil)");

}

TEST_CASE("compound") {

    BinaryExpr c{
        TokenType::star,
        std::make_unique<UnaryExpr>(
            TokenType::minus,
            std::make_unique<LiteralExpr>(123.0)
        ),
        std::make_unique<GroupedExpr>(
            std::make_unique<LiteralExpr>(45.67)
        )
    };

    CHECK(print(c) == "(* (- 123.000000) (group 45.670000))");

}





}
