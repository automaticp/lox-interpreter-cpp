#pragma once
#include "Token.hpp"
#include <variant>
#include <memory>
#include <string>
#include <utility>

class Expr;
class GroupedExpr;
class UnaryExpr;


using ExprVariant = std::variant<GroupedExpr, UnaryExpr>;


struct GroupedExpr {
    std::unique_ptr<Expr> subexpr;
};

struct UnaryExpr {
    Token op;
    std::unique_ptr<Expr> expr;
};



template <typename T, typename U>
concept not_same_as_remove_cvref =
    !std::same_as<
        std::remove_cvref_t<T>,
        std::remove_cvref_t<U>
    >;

class Expr {
private:
    ExprVariant variant_;

public:
    template<not_same_as_remove_cvref<Expr> Alternative>
    Expr(Alternative&& expr) : variant_{ std::forward<Alternative>(expr) } {}

    Expr() = delete;

    auto index() const noexcept { return variant_.index(); }

    template<typename V>
    auto accept(V&& visitor) const {
        return std::visit(std::forward<V>(visitor), variant_);
    }
};



struct ExprNameVisitor {
    std::string operator()(const GroupedExpr&) const { return "GroupedExpr"; }
    std::string operator()(const UnaryExpr&) const { return "UnaryExpr"; }
};


inline auto get_name(const Expr& expr) {
    return expr.accept(ExprNameVisitor{});
}



// This actually works!
