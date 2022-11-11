#pragma once
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <string>
#include <utility>
#include <unordered_map>


class IRGen {
private:
    llvm::LLVMContext context_;
    llvm::IRBuilder<> builder_;
    llvm::Module module_;
    std::unordered_map<std::string, llvm::Value*> env_;

    friend class IRGenVisitor;
};
