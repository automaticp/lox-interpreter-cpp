#include "Value.hpp"
#include "ExprVisitors.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "Interpreter.hpp"
#include "StmtVisitors.hpp"
#include <memory>
#include <cassert>




size_t Function::arity() const noexcept {
    assert(declaration_);
    return declaration_->parameters.size();
}

Value Function::operator()(const ExprInterpreterVisitor& interpret_visitor, std::vector<Value>& args) {
    assert(declaration_);
    // Environment from enclosing scope,
    // captured by copy during construction of Function
    Environment env{ &closure_ };

    for (size_t i{ 0 }; i < args.size(); ++i) {
        env.define(
            declaration_->parameters[i].lexeme, std::move(args[i])
        );
    }

    try {
        interpret_visitor.interpreter.interpret(
            declaration_->body, env
        );
    } catch (Value v) {
        return v;
    }

    return { nullptr };
}


Value BuiltinFunction::operator()(std::span<Value> args) {
    return fun_(args);
}






std::string ValueToStringVisitor::operator()(const Function& val) const {
    return fmt::format("?Function {}?", val.declaration_->name.lexeme);
}

std::string ValueToStringVisitor::operator()(const BuiltinFunction& val) const {
    return fmt::format("?BuiltinFunction {}?", val.name_);
}
