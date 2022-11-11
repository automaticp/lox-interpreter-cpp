#include "IRGenVisitor.hpp"
#include "IRGen.hpp"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Constants.h>


llvm::Value* IRGenVisitor::operator()(const LiteralExpr& expr) const {
    return llvm::ConstantFP::get(
        gen_.context_,
        llvm::APFloat{
            std::get<Number>(expr.token.literal())
        }
    );
}
