#pragma once
#include "ExprResolveVisitor.hpp"
#include "StmtResolveVisitor.hpp"
#include "IStmt.hpp"
#include "ErrorReporter.hpp"
#include <memory>
#include <vector>
#include <stack>
#include <boost/unordered_map.hpp>
#include <string>
#include <span>

class IExpr;


enum class ResolveState : bool {
    declared = false,
    defined = true
};

class Resolver {
private:
    ErrorReporter& err_;
    StmtResolveVisitor visitor_;

    using map_t = boost::unordered_map<std::string, ResolveState>;
    using scope_stack_t = std::vector<map_t>;
    scope_stack_t scope_stack_;

    using depth_map_t = boost::unordered_map<const IExpr*, size_t>;
    depth_map_t depth_map_;

public:
    Resolver(ErrorReporter& err) :
        err_{ err }, visitor_{ *this, err } {}


    void resolve(std::span<std::unique_ptr<IStmt>> stmts) {
        for (const auto& stmt : stmts) {
            stmt->accept(visitor_);
        }
    }

    void push_scope() {
        scope_stack_.emplace_back();
    }

    void pop_scope() {
        scope_stack_.pop_back();
    }

    map_t& top_scope() {
        return scope_stack_.back();
    }

    map_t& scope_at(size_t idx) {
        return scope_stack_[idx];
    }

    scope_stack_t& scopes() {
        return scope_stack_;
    }

    bool is_in_global_scope() const noexcept {
        return scope_stack_.empty();
    }

    void declare(const std::string& name) {
        top_scope()[name] = ResolveState::declared;
    }

    void define(const std::string& name) {
        top_scope()[name] = ResolveState::defined;
    }


    void set_depth(const IExpr& expr, size_t depth) {
        depth_map_[&expr] = depth;
    }

    depth_map_t& depth_map() {
        return depth_map_;
    }

};