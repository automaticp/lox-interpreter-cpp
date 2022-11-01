#pragma once
#include <cstddef>
#include <memory>
#include <array>



template<typename ...Visitors>
struct IAVisitable;

// Recursive inheritance
template<typename Visitor, typename ...OtherVisitors>
struct IAVisitable<Visitor, OtherVisitors...> :
    IAVisitable<OtherVisitors...> {

    // Reexpose to prevent shadowing.
    // (Is this neccessary for virtual functions?)
    using IAVisitable<OtherVisitors...>::accept;

    // Add pv interface for this visitor.
    virtual typename Visitor::return_type
    accept(const Visitor& visitor) const = 0;

};

// Tail of the type sequence. Instantiated first.
// No further inheritance. Protected destructor.
template<typename LastVisitor>
struct IAVisitable<LastVisitor> {

    virtual typename LastVisitor::return_type
    accept(const LastVisitor& visitor) = 0;

protected:
    // Must not be deleted through any of the IAVisitor interfaces
    ~IAVisitable() = default;
};




template<typename AdapteeT, typename ...Visitors>
struct AVisitable;

template<typename AdapteeT, typename Interface, typename Visitor, typename ...OtherVisitors>
struct AVisitable<AdapteeT, Interface, Visitor, OtherVisitors...> :
    AVisitable<AdapteeT, Interface, OtherVisitors...> {

    using Interface::adaptee;

    virtual typename Visitor::return_type
    accept(const Visitor& visitor) const {
        return visitor(static_cast<const AdapteeT&>(adaptee));
    }

};


template<typename AdapteeT, typename Interface, typename LastVisitor>
struct AVisitable<AdapteeT, Interface, LastVisitor> : Interface {

    using Interface::adaptee;

    virtual typename LastVisitor::return_type
    accept(const LastVisitor& visitor) const {
        return visitor(static_cast<const AdapteeT&>(adaptee));
    }

};













// I'll call this "Static Dynamic External Polymorphic Visitor pattern".
// aka I'm too cheap to do dynamic allocations just for polymorphism.

template<typename AdapteeT, typename ...Visitors>
class IVAdapter : public IAVisitable<Visitors...> {
private:
    const AdapteeT& adaptee;
protected:
    // No public c-tor yet
    IVAdapter(const AdapteeT& adaptee) : adaptee{ adaptee } {}
public:
    // But a public d-tor - suitable base class
    virtual ~IVAdapter() = default;
};


template<typename AdapteeT /* tee-tee, hehe */, typename ...Visitors>
class VAdapter : public IVAdapter<Visitors...> {
private:
    const AdapteeT& adaptee_;
public:
    VAdapter(const AdapteeT& adaptee) : adaptee_{ adaptee } {}
};














// The "Virtual Override Sandwich" pattern:

class IAdapterFactory {
public:
    virtual const IVAdapter* construct_adapter_at(void*) const = 0;
protected:
    ~IAdapterFactory() = default;
};

class Expr : public IAdapterFactory {
public:
    virtual ~Expr() = default;
};

template<typename CRTP, typename Base>
class AdapterFactory : public Base {
public:
    const IVAdapter* construct_adapter_at(void* location) const override {
        return std::construct_at<VAdapter<CRTP>>(
            static_cast<VAdapter<CRTP>*>(location),
            static_cast<const CRTP&>(*this)
        );
    }
};

// For example
class GroupedExpr : public AdapterFactory<GroupedExpr, IExpr> {
public:
    std::unique_ptr<Expr> subexpr;
};


// The main idea is that the concrete VAdapter<> size is going to be fixed.
// On 64 bit platforms it's going to be equal to 16 bytes, as the total size of
// the vtable pointer and the pointer to the adaptee.
// Thus, instead of allocating the adapter dynamically, we can create in a local
// buffer on the stack.
// Dynamic allocations are undesirable and largely unneccesary, as the lifetime
// of the created adapter is unlikely to exceed a scope of a single statement.



// This is a raw usage example:

class Value {};

inline Value evaluate(const Expr& expr) {
    // Normally, you'd have something like
    //
    // expr.accept(ExprInterpretVisitor{});
    //
    // With External Polymorphism you'd have to do a
    // heap allocation in order to use polymorphism
    // within the lifetime of the adapter.
    //
    //
    //
    // But we'll do a funny:

    alignas(void*) std::array<std::byte, 2*sizeof(void*)> adapter_buffer; // <-- Storage is here
    const IVAdapter* adapter =                             // <-- Pointer to base is here
        expr.construct_adapter_at(&adapter_buffer);        // The lifetime is started for a concrete type

    Value result{
        adapter->accept(ExprInterpreterVisitor{}) // Do external polymorphism adapter things
    };

    // End the lifetime of the adapter through the virtual destructor
    adapter->~IVAdapter();
    // or, alternitevly:
    // std::destroy_at(adapter);

    // Then return the result
    return result;
}


inline Value interpret_visitor_call(const GroupedExpr& expr) {
    return evaluate(*expr.subexpr);
}




// This doesn't actually work.
// The visitor pack is still part of the expression types.
