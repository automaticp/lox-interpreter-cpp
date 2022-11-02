#include <doctest/doctest.h>
#include <memory>
#include "CommonVisitors.hpp"
#include "Expr.hpp"
#include "Token.hpp"
#include "TokenType.hpp"


static auto print(const Expr& expr) {
    return expr.accept(ASTPrintVisitor{});
}


TEST_SUITE("ASTPrinter") {

TEST_CASE("literals") {

    using enum TokenType;

    LiteralExpr l_string{ Token{ string, "\"hello\"", 0, "hello" } };
    LiteralExpr l_number{ Token{ number, "5.6", 0, 5.6 } };
    LiteralExpr l_boolean{ Token{ kw_true, "true", 0, true } };
    LiteralExpr l_nil{ Token{ kw_nil, "nil", 0, nullptr } };

    CHECK(print(l_string)   == "\"hello\"");
    CHECK(print(l_number)   == "5.6");
    CHECK(print(l_boolean)  == "true");
    CHECK(print(l_nil)      == "nil");

}


TEST_CASE("unary") {

    using enum TokenType;

    UnaryExpr u_not_true{
        Token{ bang, "!", 0 },
        Expr::make_unique<LiteralExpr>(Token{ kw_true, "true", 0, true})
    };

    UnaryExpr u_minus_num{
        Token{ minus, "-", 0 },
        Expr::make_unique<LiteralExpr>(Token{ number, "5.6", 0, 5.6 })
    };

    CHECK(print(u_not_true) == "(! true)");
    CHECK(print(u_minus_num) == "(- 5.6)");

}


TEST_CASE("binary") {

    using enum TokenType;

    BinaryExpr b_num_times_num{
        Token{ star, "*", 0 },
        Expr::make_unique<LiteralExpr>(Token{ number, "5.6", 0,5.6 }),
        Expr::make_unique<LiteralExpr>(Token{ number, "0.1", 0, 0.1 })
    };

    BinaryExpr b_false_and_true{
        Token{ kw_and, "and", 0 },
        Expr::make_unique<LiteralExpr>(Token{ kw_false, "false", 0, false }),
        Expr::make_unique<LiteralExpr>(Token{ kw_true, "true", 0, true})
    };

    BinaryExpr b_hello_plus_world{
        Token{ plus, "+", 0 },
        Expr::make_unique<LiteralExpr>(Token{ string, "\"hello\"", 0, "hello" }),
        Expr::make_unique<LiteralExpr>(Token{ string, "\"world\"", 0, "world" })
    };

    CHECK(print(b_num_times_num) == "(* 5.6 0.1)");
    CHECK(print(b_false_and_true) == "(and false true)");
    CHECK(print(b_hello_plus_world) == "(+ \"hello\" \"world\")");

}


TEST_CASE("group") {

    GroupedExpr g_nil{
        Expr::make_unique<LiteralExpr>(Token{ TokenType::kw_nil, "nil", 0, nullptr })
    };

    CHECK(print(g_nil) == "(group nil)");

}

TEST_CASE("compound") {

    using enum TokenType;

    BinaryExpr c{
        Token{ star, "*", 0},
        Expr::make_unique<UnaryExpr>(
            Token{ minus, "-", 0 },
            Expr::make_unique<LiteralExpr>(Token{ number, "123", 0, 123.0 } )
        ),
        Expr::make_unique<GroupedExpr>(
            Expr::make_unique<LiteralExpr>(Token{ number, "45.67", 0, 45.67 })
        )
    };

    CHECK(print(c) == "(* (- 123) (group 45.67))");

}





}
