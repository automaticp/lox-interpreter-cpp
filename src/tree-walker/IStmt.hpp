#pragma once
#include "IVisitable.hpp"
#include "StmtVisitors.hpp"

// Design notes:
//
// See IExpr.hpp and IVisitable.hpp.

using IFullyVisitableStmt = IVisitable<
    StmtInterpreterVisitor, StmtASTPrinterVisitor, StmtResolveVisitor
>;

struct IStmt : IFullyVisitableStmt {
public:
    virtual ~IStmt() = default;
};

template<typename CRTP>
using FullyVisitableStmt = Visitable<
    CRTP, IStmt, StmtInterpreterVisitor, StmtASTPrinterVisitor, StmtResolveVisitor
>;
