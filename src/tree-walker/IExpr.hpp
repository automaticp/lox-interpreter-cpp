#pragma once
#include "ExprVisitors.hpp"
#include "IVisitable.hpp"

// Design notes:
//
// See IVisitable.hpp for description of IVisitable and Visitable
// and this file for their usage.


// The following alias defines a type with the full overload set
// of pure virtual accept() methods for all the visitors listed
// in the template parameter pack.
//
// Extend this list (and the list below) to add more visitors.
using IFullyVisitableExpr = IVisitable<
    ExprASTPrinterVisitor, ExprInterpreterVisitor, ExprGetPrimaryTokenVisitor, ExprUserFriendlyNameVisitor
>;


// IExpr inherits the 'accept() interface' and defines a public virtual destructor,
// making it a suitable base class. This is an 'expression interface'.
struct IExpr : IFullyVisitableExpr {
public:
    virtual ~IExpr() = default;
};


// Finlly we define an alias template that introduces a set
// of virtual accept() methods, with the visited type as the template parameter.
//
// Extend this list to add more visitors.
template<typename CRTP>
using FullyVisitableExpr = Visitable<
    CRTP, IExpr, ExprASTPrinterVisitor, ExprInterpreterVisitor, ExprGetPrimaryTokenVisitor, ExprUserFriendlyNameVisitor
>;


// And now we can define concrete Derived expression classes, that implement
// the visitor pattern by simply inheriting from FullyVisitableExpr<Derived>.
//
// If it isn't clear, the complete hierarchy looks like this:
//
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
// Visitable<DerivedExpr, IExpr, Visitor1>
//     + accept(Visitor1&) { ... }
// ^
// Visitable<DerivedExpr, IExpr, Visitor1, Visitor2>
//     + accept(Visitor2&) { ... }
// ^
// ...
// ^
// Visitable<DerivedExpr, IExpr, Visitor1, Visitor2, ..., VisitorN>
//     + accept(VisitorN&) { ... }
// ^
// DerivedExpr
//
//
