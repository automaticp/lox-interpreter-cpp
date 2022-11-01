#pragma once
#include "Utils.hpp"
#include <variant>
#include <memory>
#include <utility>
#include <concepts>

template<typename ConcreteWrapper>
class WrapperBackreference;

template<typename Alternative, typename WrapperT>
concept backreferenceable_to = requires(const Alternative& alt) {
    requires(std::derived_from<Alternative, WrapperBackreference<WrapperT>>);
    { alt.get_wrapper() } -> std::same_as<WrapperT*>;
};



// A common VariantWrapper base that supplies:
//
// - Converting constructor and one of the in-place constructors;
// - is<Alternative>() type checking method (std::holds_alternative<Alt>);
// - as<Alternative>() type casting method (std::get<Alt>);
// - an accept() method for visitors;
// - a static make_unique<Alternative>() factory.
// - a static 'pointer to base'-like conversion for compliant alternatives.
//
// Derive publicly or else I'm not responsible.
//
// Also re-expose the constructors via:
//
// using VariantWrapper<...>::VariantWrapper;
//
template<typename CRTP, typename VariantT>
class VariantWrapper {
protected:
    VariantT variant_;

    // If alternative type implements backref interface:
    template<std::derived_from<WrapperBackreference<CRTP>> Alternative>
    void set_this_as_backreference_impl(Alternative& alt) {
        alt.set_wrapper(static_cast<CRTP*>(this));
    }
    // Otherwise (less constrained):
    void set_this_as_backreference_impl(auto& alt) {}


    void set_this_as_backreference() {
        // If the alternative implements the backreference interface,
        // set 'this' as the backreference.
        std::visit([this](auto& alt) { set_this_as_backreference_impl(alt); }, variant_);
    }

public:
    // Converting constructor
    template<not_derived_from_remove_cvref<VariantWrapper> Alternative>
    // NOLINTNEXTLINE(bugprone-forwarding-reference-overload): constraint above
    VariantWrapper(Alternative&& expr) :
        variant_{ std::forward<Alternative>(expr) }
    {
        set_this_as_backreference();
    }

    // Inplace constructor
    template<typename Alternative, typename ...Args>
    VariantWrapper(std::in_place_type_t<Alternative> inplace, Args&&... args) :
        variant_( inplace, std::forward<Args>(args)... )
    {
        set_this_as_backreference();
    }

    // Default constructor because maybe you need one
    VariantWrapper() = default;


    auto index() const noexcept { return variant_.index(); }


    // Accept visitor
    template<visitor_of<VariantT> V>
    auto accept(V&& visitor) const {
        return std::visit(std::forward<V>(visitor), variant_);
    }


    // Type checking
    template<typename T>
    bool is() const noexcept {
        return std::holds_alternative<T>(variant_);
    }

    template<typename ...Ts>
    bool is_any_of() const noexcept {
        return (... || is<Ts>());
    }

    // Type casting / variant accessing
    template<typename T>
    T& as() & { return std::get<T>(variant_); }

    template<typename T>
    T as() && { return std::move(std::get<T>(variant_)); }

    template<typename T>
    const T& as() const& { return std::get<T>(variant_); }


    // This exists, because unlike the Derinved* -> Base* conversion,
    // the Alternative -> Variant conversion is not implicit, and is not possible
    // in case std::unique_ptr<Alternative> -> std::unique_ptr<Variant>.
    //
    // Usage example:
    //
    //     struct Expr : VariantWrapper<Expr, ExprVariant> {};
    //
    //     std::unique_ptr<Expr> =
    //         Expr::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
    //
    template<typename Alternative, typename ...Args>
    static std::unique_ptr<CRTP> make_unique(Args&&... args) {
        // Use inplace consturctor
        return std::make_unique<CRTP>(std::in_place_type<Alternative>, std::forward<Args>(args)...);
    }
    // Maybe use boost::variant and boost::make_recursive_variant instead.
    // The idea is similar, at least...



    // Creates an lvalue reference to the wrapper,
    // that owns the alternative 'alt'.
    // Requires that each variant alternative implements
    // the WrapperBackreference interface (see below).
    template<backreferenceable_to<CRTP> Alternative>
    static CRTP& from_alternative(const Alternative& alt) {
        assert(alt.get_wrapper());
        return *alt.get_wrapper();
    }
    // Kind of hacky, but gets you out of trouble when
    // you need, for example, an Expr& but only have CallExpr&.
    // Similar to casting to 'pointer to base', but worse,
    // because there's no 'base' in a variant.
};



// A simple helper base type, that adds a backreference
// to a wrapper class for each alternative.

template<typename ConcreteWrapper>
struct WrapperBackreference {
private:
    ConcreteWrapper* owning_wrapper_{ nullptr };

public:
    void set_wrapper(ConcreteWrapper* wrapper) noexcept {
        owning_wrapper_ = wrapper;
    }

    bool has_wrapper() const noexcept { return owning_wrapper_; }

    ConcreteWrapper* get_wrapper() const noexcept {
        return owning_wrapper_;
    }

    operator ConcreteWrapper&() noexcept {
        assert(owning_wrapper_);
        return *owning_wrapper_;
    }

    operator const ConcreteWrapper&() const noexcept {
        assert(owning_wrapper_);
        return *owning_wrapper_;
    }
};
