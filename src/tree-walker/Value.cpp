#include "Value.hpp"
#include "ObjectImpl.hpp"
#include "ExprVisitors.hpp"
#include "Environment.hpp"
#include "Stmt.hpp"
#include "Interpreter.hpp"
#include <memory>
#include <cassert>


Object::Object() : pimpl_{ std::make_unique<ObjectImpl>() } {}
Object& Object::operator=(Object &&) = default;
Object::Object(Object &&) = default;
Object::~Object() = default;


Object::Object(const Object& other) :
    pimpl_{ std::make_unique<ObjectImpl>(other.impl()) } {}

Object& Object::operator=(const Object& other) {
    if (this != &other) {
        pimpl_ = std::make_unique<ObjectImpl>(other.impl());
    }
    return *this;
}


size_t Function::arity() const noexcept {
    assert(declaration_);
    return declaration_->parameters.size();
}

Value Function::operator()(const ExprInterpreterVisitor& interpret_visitor, std::vector<Value>& args) {
    assert(declaration_);
    // Environment from enclosing scope, not just from global
    Environment env{ &interpret_visitor.env };

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
