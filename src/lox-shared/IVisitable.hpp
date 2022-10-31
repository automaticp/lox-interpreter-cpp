#pragma once

// Design notes:
//
// (Also see IExpr.hpp for concrete implementation of this design).
//
// We are trying to implement a set of expression (and statement) types that share a common base
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




// The following code produces an overload set for the implementations
// of the accept() methods. Since for each derived expression type these sets
// will only differ by the type of the 'this' pointer, we can use the CRTP idiom here.
//
// Keep in mind that the instantiated Visitable type is not independent,
// but rather is a type derived from some common base Interface.
// This is the only way to guarantee that the pure virtual accept() methods will be overriden.
//
// Unfortunately, 'parallel overriding' via multiple inheritance, as in:
//
//     struct Concrete : Interface, Visitable<Visitor1, Visitor2, ...> {};
//
// does not seem to work, and instead introduces an ambiguity between
// accept() methods from IVisitable and the ones from Visitable.
// (Could be that I'm missing something, tbh)

template<typename CRTP, typename Interface, typename ...Visitors>
struct Visitable;

template<typename CRTP, typename Interface, typename Visitor, typename ...OtherVisitors>
struct Visitable<CRTP, Interface, Visitor, OtherVisitors...> :
    Visitable<CRTP, Interface, OtherVisitors...> {

    virtual typename Visitor::return_type accept(const Visitor& visitor) const {
        return visitor(static_cast<const CRTP&>(*this));
    }

    friend Visitor;

};

// Here we 'hook' into the Interface inheritance hierarchy
// by inheriting from Interface on the first instantiation.
template<typename CRTP, typename Interface, typename LastVisitor>
struct Visitable<CRTP, Interface, LastVisitor> : Interface {

    virtual typename LastVisitor::return_type accept(const LastVisitor& visitor) const {
        return visitor(static_cast<const CRTP&>(*this));
    }

    friend LastVisitor;

};


















