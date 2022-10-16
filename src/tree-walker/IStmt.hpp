#pragma once
#include "IVisitable.hpp"


// Design notes:
//
// See IExpr.hpp and IVisitable.hpp.

struct StmtVisitor {
    using return_type = void;
};

using IFullyVisitableStmt = IVisitable<
    StmtVisitor
>;

class IStmt : IFullyVisitableStmt {
public:
    virtual ~IStmt() = default;
};

template<typename CRTP>
using FullyVisitableStmt = Visitable<
    CRTP, IStmt, StmtVisitor
>;
