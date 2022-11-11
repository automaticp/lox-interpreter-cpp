#pragma once
#include "ErrorSender.hpp"
#include "IError.hpp"
#include "IRGenVisitor.hpp"
#include "Token.hpp"
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <string>
#include <span>
#include <memory>
#include <utility>
#include <unordered_map>


// Dummy, replace later
class SimpleIRGenError : public IError {
private:
    std::string message_;

public:
    SimpleIRGenError(std::string message) :
        message_{ std::move(message) }
    {}

    std::string message() const override {
        return message_;
    }

    ErrorCategory category() const override {
        return ErrorCategory::interpreter;
    }
};

class IRGen : private ErrorSender<SimpleIRGenError> {
private:
    llvm::LLVMContext context_;
    llvm::IRBuilder<> builder_;
    llvm::Module module_;
    std::unordered_map<std::string, llvm::Value*> env_;

    friend class IRGenVisitor;
public:
    IRGen(ErrorReporter& err) :
        ErrorSender{ err },
        context_{},
        builder_{ context_ },
        module_{ "lox-jit", context_ }
    {}

    void generate(std::span<std::unique_ptr<Stmt>> stmts) {
        IRGenVisitor visitor{ *this };
        for (const auto& stmt : stmts) {
            stmt->accept(visitor);
        }
    }

    llvm::Module& module() noexcept { return module_; }
    const llvm::Module& module() const noexcept { return module_; }

    llvm::IRBuilder<> builder() noexcept { return builder_; }

};
