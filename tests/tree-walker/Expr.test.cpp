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

    // It's neccessary to wrap them in Expr in order to set the backref.
    Expr l_string{ LiteralExpr{ Token{ string, "\"hello\"", {0, 1}, "hello" } } };
    Expr l_number{ LiteralExpr{ Token{ number, "5.6", {0, 1}, 5.6 } } };
    Expr l_boolean{ LiteralExpr{ Token{ kw_true, "true", {0, 1}, true } } };
    Expr l_nil{ LiteralExpr{ Token{ kw_nil, "nil", {0, 1}, Nil{} } } };

    CHECK(print(l_string)   == "\"hello\"");
    CHECK(print(l_number)   == "5.6");
    CHECK(print(l_boolean)  == "true");
    CHECK(print(l_nil)      == "nil");

}


TEST_CASE("unary") {

    using enum TokenType;

    Expr u_not_true{ UnaryExpr{
        Token{ bang, "!", {0, 1} },
        Expr::make_unique<LiteralExpr>(Token{ kw_true, "true", {0, 1}, true})
    } };

    Expr u_minus_num{ UnaryExpr{
        Token{ minus, "-", {0, 1} },
        Expr::make_unique<LiteralExpr>(Token{ number, "5.6", {0, 1}, 5.6 })
    } };

    CHECK(print(u_not_true) == "(! true)");
    CHECK(print(u_minus_num) == "(- 5.6)");

}


TEST_CASE("binary") {

    using enum TokenType;

    Expr b_num_times_num{ BinaryExpr{
        Token{ star, "*", {0, 1} },
        Expr::make_unique<LiteralExpr>(Token{ number, "5.6", {0, 1} ,5.6 }),
        Expr::make_unique<LiteralExpr>(Token{ number, "0.1", {0, 1}, 0.1 })
    } };

    Expr b_false_and_true{ BinaryExpr{
        Token{ kw_and, "and", {0, 1} },
        Expr::make_unique<LiteralExpr>(Token{ kw_false, "false", {0, 1}, false }),
        Expr::make_unique<LiteralExpr>(Token{ kw_true, "true", {0, 1}, true})
    } };

    Expr b_hello_plus_world{ BinaryExpr{
        Token{ plus, "+", {0, 1} },
        Expr::make_unique<LiteralExpr>(Token{ string, "\"hello\"", {0, 1}, "hello" }),
        Expr::make_unique<LiteralExpr>(Token{ string, "\"world\"", {0, 1}, "world" })
    } };

    CHECK(print(b_num_times_num) == "(* 5.6 0.1)");
    CHECK(print(b_false_and_true) == "(and false true)");
    CHECK(print(b_hello_plus_world) == "(+ \"hello\" \"world\")");

}


TEST_CASE("group") {

    Expr g_nil{ GroupedExpr{
        Expr::make_unique<LiteralExpr>(Token{ TokenType::kw_nil, "nil", {0, 1}, Nil{} })
    } };

    CHECK(print(g_nil) == "(group nil)");

}

TEST_CASE("compound") {

    using enum TokenType;

    Expr c{ BinaryExpr{
        Token{ star, "*", {0, 1} },
        Expr::make_unique<UnaryExpr>(
            Token{ minus, "-", {0, 1} },
            Expr::make_unique<LiteralExpr>(Token{ number, "123", {0, 1}, 123.0 } )
        ),
        Expr::make_unique<GroupedExpr>(
            Expr::make_unique<LiteralExpr>(Token{ number, "45.67", {0, 1}, 45.67 })
        )
    } };

    CHECK(print(c) == "(* (- 123) (group 45.67))");

}





}
