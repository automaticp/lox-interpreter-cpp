#pragma once


// If inherited from, the following classes
// add accept() overloads
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

