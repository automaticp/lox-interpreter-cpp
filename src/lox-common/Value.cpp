#include "Value.hpp"
#include "CommonVisitors.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "ValueDecl.hpp"
#include <memory>
#include <cassert>




ValueHandle::ValueHandle(Value& target) noexcept : handle_{ &target } {
    assert(
        !target.is<ValueHandle>() &&
        "Handle to a Handle is redundant"
    );
}

Value ValueHandle::decay() const noexcept {
    assert(handle_);
    return *handle_;
}




size_t Function::arity() const noexcept {
    assert(declaration_);
    return declaration_->parameters.size();
}






namespace detail {

std::string ValueToStringVisitor::operator()(const ValueHandle& val) const {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): Find me a better way, yeah
    return fmt::format("?ValueHandle 0x{:x}?", reinterpret_cast<uintptr_t>(val.pointer()));
}

std::string ValueToStringVisitor::operator()(const Function& val) const {
    return fmt::format("?Function {}?", val.declaration_->name.lexeme);
}

std::string ValueToStringVisitor::operator()(const BuiltinFunction& val) const {
    return fmt::format("?BuiltinFunction {}?", val.name_);
}

} // namespace detail
