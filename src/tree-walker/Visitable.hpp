#pragma once





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
// A1:The template instantiations between the interface Expr class
//    and its concrete derived classes will not be syncronized.
//    Instantiations will only happen when they are invoked somewhere.
//    In practice, all the invokations happen through the base class pointer,
//    which means that the accept() methods will only be instantiated for
//    the base class, leaving the derived classes without the required overrides.
//
// A2:Function templates cannot be declared virtual.




