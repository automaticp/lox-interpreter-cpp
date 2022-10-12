#pragma once
#include <variant>
#include <utility>
#include <memory>
#include <string>
#include <sstream>
#include <concepts>
#include "LiteralValue.hpp"
#include "Token.hpp"
#include "ExprVisitors.hpp"



// Design notes:
//
// We are trying to implement a set of expression types that share a common base
// and also implement the visitor pattern. We also have to enable different visitors
// to produce values of different return types.
//
// We need:
// - A base class that declares an overload set of pure virtual accept methods;
// - Derived classes that implement these methods.
//
// We cannot use a single IVisitor interface class and define only one method:
//
//     return_type accept(const IVisitor &) const;
//
// as this makes it impossible to have different return types for different visitors.
// Simulating dynamic return types will require even more work.
// Retured value instead could be stored inside the concrete Visitor, but that also
// entails the annoyance of unpacking and a mutability requirement for the Visitor.
//
// Thus, our 'accept() interface' has to be an overload set:
//
//     virtual Visitor1::return_type accept(const Visitor1 &) const = 0;
//     virtual Visitor2::return_type accept(const Visitor2 &) const = 0;
//     ...
//     virtual Visitor3::return_type accept(const VisitorN &) const = 0;
//
// Writing this overload set by hand is quite tedious and error-prone,
// so we use the magic of ~~template metaprogramming~~ to help with the task.
//
// Instantiating the followind IVisitable template produces this overload set
// for the visitor types listed as the template arguments.
// The visitor classes are only reqired to define the 'return_type' alias.

template<typename ...Visitors>
struct IVisitable;

template<typename Visitor, typename ...OtherVisitors>
struct IVisitable<Visitor, OtherVisitors...> :
    IVisitable<OtherVisitors...> {

    using IVisitable<OtherVisitors...>::accept;

    virtual typename Visitor::return_type accept(const Visitor& visitor) const = 0;

};

// Tail of the type sequence. Instantiated first.
template<typename LastVisitor>
struct IVisitable<LastVisitor> {

    virtual typename LastVisitor::return_type accept(const LastVisitor& visitor) const = 0;

protected:
    // Must not be deleted through any of the IVisitor interfaces
    ~IVisitable() = default;
};




// The following alias defines a type with the full overload set
// of pure virtual accept() methods for all the visitors listed
// in the template parameter pack.
//
// Extend this list (and the list below) to add more visitors.
using IFullyVisitableExpr = IVisitable<
    ExprASTPrinterVisitor
>;


// IExpr inherits the 'accept() interface' and defines a public virtual destructor,
// making it a suitable base class. This is finally an 'expression interface'.
struct IExpr : IFullyVisitableExpr {
public:
    virtual ~IExpr() = default;
};




// The following code produces an overload set for the implementations
// of the accept() methods. Since for each derived expression type these sets
// will only differ by the type of the 'this' pointer, we can use the CRTP idiom here.
//
// Keep in mind that the instantiated Visitable type is not independent,
// but rather is a type derived from IExpr. This is the only way to guarantee
// that the pure virtual accept() methods will be overriden.
//
// Unfortunately, 'parallel overriding' via multiple inheritance, as in:
//
//     struct Expr : IExpr, Visitable<Visitor1, Visitor2, ...> {};
//
// does not seem to work, and instead introduces an ambiguity between
// accept() methods from IVisitable and the ones from Visitable.
// (Could be that I'm missing something, tbh)

template<typename CRTP, typename ...Visitors>
struct Visitable;

template<typename CRTP, typename Visitor, typename ...OtherVisitors>
struct Visitable<CRTP, Visitor, OtherVisitors...> :
    Visitable<CRTP, OtherVisitors...> {

    /* virtual */ typename Visitor::return_type accept(const Visitor& visitor) const {
        return visitor(static_cast<const CRTP&>(*this));
    }

    friend Visitor;

};

// Here we 'hook' into the IExpr inheritance hierarchy
// by inheriting from IExpr on the first instantiation.
template<typename CRTP, typename LastVisitor>
struct Visitable<CRTP, LastVisitor> : IExpr {

    /* virtual */ typename LastVisitor::return_type accept(const LastVisitor& visitor) const {
        return visitor(static_cast<const CRTP&>(*this));
    }

    friend LastVisitor;

};




// Finlly we define an alias template that introduces a set
// of virtual accept() methods, with the visited type as the template parameter.
//
// Extend this list to add more visitors.
template<typename CRTP>
using FullyVisitableExpr = Visitable<
    CRTP, ExprASTPrinterVisitor
>;


// And now we can define concrete Derived expression classes, that implement
// the visitor pattern by simply inheriting from FullyVisitableExpr<Derived>.
//
// If it isn't clear, the complete hierarchy looks like this:
//
// IVisitable<Visitor1>
//     + accept(Visitor1&) = 0;
// ^
// IVisitable<Visitor1, Visitor2>
//     + accept(Visitor2&) = 0;
// ^
// ...
// ^
// IVisitable<Visitor1, Visitor2, ..., VisitorN>
//     + accept(VisitorN&) = 0;
// ^
// IExpr
//     + public virtual ~IExpr() = default;
// ^
// Visitable<DerivedExpr, Visitor1>
//     + accept(Visitor1&) { ... }
// ^
// Visitable<DerivedExpr, Visitor1, Visitor2>
//     + accept(Visitor2&) { ... }
// ^
// ...
// ^
// Visitable<DerivedExpr, Visitor1, Visitor2, ..., VisitorN>
//     + accept(VisitorN&) { ... }
// ^
// DerivedExpr
//


struct LiteralExpr : FullyVisitableExpr<LiteralExpr> {
public:
    LiteralValue value;

    LiteralExpr(LiteralValue value) :
        value{ std::move(value) } {}
};


struct UnaryExpr : FullyVisitableExpr<UnaryExpr> {
public:
    TokenType op;
    std::unique_ptr<IExpr> operand;

    UnaryExpr(TokenType op, std::unique_ptr<IExpr> expr) :
        op{ op }, operand{ std::move(expr) } {}
};


struct BinaryExpr : FullyVisitableExpr<BinaryExpr> {
public:
    TokenType op;
    std::unique_ptr<IExpr> lhs;
    std::unique_ptr<IExpr> rhs;

    BinaryExpr(TokenType op, std::unique_ptr<IExpr> lhs, std::unique_ptr<IExpr> rhs) :
        op{ op }, lhs{ std::move(lhs) }, rhs{ std::move(rhs) } {}
};


struct GroupedExpr : FullyVisitableExpr<GroupedExpr> {
public:
    std::unique_ptr<IExpr> expr;

    GroupedExpr(std::unique_ptr<IExpr> expr) :
        expr{ std::move(expr) } {}
};
