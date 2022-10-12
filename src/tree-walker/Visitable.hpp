#pragma once


// If inherited from, the following classes add accept() overloads
// for all the visitors listed in ...Visitors
// with the correct return types

// IVisitableExpr adds pure vurtual interfaces

template<typename CRTP, typename ...Visitors>
struct IVisitable;

template<typename CRTP, typename Visitor, typename ...OtherVisitors>
struct IVisitable<CRTP, Visitor, OtherVisitors...> :
    IVisitable<CRTP, OtherVisitors...> {

    using IVisitable<CRTP, OtherVisitors...>::accept;

    virtual typename Visitor::return_type accept(const Visitor& visitor) const = 0;

};

template<typename CRTP, typename LastVisitor>
struct IVisitable<CRTP, LastVisitor> {

    virtual typename LastVisitor::return_type accept(const LastVisitor& visitor) const = 0;

};




// VisitableExpr adds implementations

template<typename CRTP, typename ...Visitors>
struct Visitable;

template<typename CRTP, typename Visitor, typename ...OtherVisitors>
struct Visitable<CRTP, Visitor, OtherVisitors...> :
    Visitable<CRTP, OtherVisitors...> {

    using Visitable<CRTP, OtherVisitors...>::accept;

    virtual typename Visitor::return_type accept(const Visitor& visitor) const {
        return visitor(static_cast<const CRTP&>(*this));
    }

    friend Visitor;

};


template<typename CRTP, typename LastVisitor>
struct Visitable<CRTP, LastVisitor> {

    virtual typename LastVisitor::return_type accept(const LastVisitor& visitor) const {
        return visitor(static_cast<const CRTP&>(*this));
    }

    friend LastVisitor;

};


// Implementation FAQ
//
// Q: Why not have some IVisitor interface class and define only one
//
//        return_type accept(const IVisitor&) const;
//
//    method?
//
// A: Makes it impossible to have different return types for different visitors,
//    as the return type will be inquired from the parameter type (IVisitor).
//    Simulating dynamic return types will require even more work.
//    Retured value instead could be stored inside the Visitor, but that also
//    entails annoyance of unpacking and mutability requirement for the Visitor.
//
// Q: Why not just define a single templated method
//
//        template<typename V>
//        auto accept(const V& visitor) const {
//            return visitor(static_cast<const CRTP&>(*this));
//        }
//
//    in the Visitable base class instead of unpacking
//    an explicit list of visitor types?
//
// A: The template instantiations between the interface Expr class
//    and its concrete derived classes will not be syncronized.
//    Instantiations will only happen when they are invoked somewhere.
//    In practice, all the invokations happen through the base class pointer,
//    which means that the accept() methods will only be instantiated for
//    the base class, leaving the derived classes without the required overrides.


