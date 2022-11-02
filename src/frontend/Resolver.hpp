#pragma once
#include "ExprResolveVisitor.hpp"
#include "StmtResolveVisitor.hpp"
#include "Stmt.hpp"
#include "ErrorSender.hpp"
#include "FrontendErrors.hpp"
#include "ErrorReporter.hpp"
#include <memory>
#include <vector>
#include <stack>
#include <boost/unordered_map.hpp>
#include <string>
#include <span>

class Expr;


enum class ResolveState : bool {
    declared = false,
    defined = true
};

enum class ScopeType {
    global,
    block,
    function
};



class Resolver : private ErrorSender<ResolverError> {
public:
    using map_t = boost::unordered_map<std::string, ResolveState>;
    using scope_stack_t = std::vector<map_t>;
    using scope_type_stack_t = std::vector<ScopeType>;
    using depth_map_t = boost::unordered_map<const Expr*, size_t>;

private:
    StmtResolveVisitor visitor_;
    friend StmtResolveVisitor;
    friend ExprResolveVisitor;

    scope_stack_t scope_stack_;
    scope_type_stack_t scope_type_stack_;
    depth_map_t depth_map_;

    bool is_in_function_prev_{ false };
    bool is_in_function_{ false };

public:
    Resolver(ErrorReporter& err) :
        ErrorSender{ err }, visitor_{ *this } {
            push_scope(ScopeType::global);
        }


    void resolve(std::span<std::unique_ptr<Stmt>> stmts) {
        for (const auto& stmt : stmts) {
            stmt->accept(visitor_);
        }
    }

    void push_scope(ScopeType type) {
        if (type == ScopeType::function) {
            is_in_function_prev_ = is_in_function_;
            is_in_function_ = true;
        }

        scope_type_stack_.emplace_back(type);
        scope_stack_.emplace_back();
    }

    void pop_scope() {
        if (top_scope_type() == ScopeType::function) {
            is_in_function_ = is_in_function_prev_;
        }

        scope_type_stack_.pop_back();
        scope_stack_.pop_back();
    }

    map_t& top_scope() {
        return scope_stack_.back();
    }

    ScopeType& top_scope_type() {
        return scope_type_stack_.back();
    }

    const ScopeType& top_scope_type() const {
        return scope_type_stack_.back();
    }


    map_t& scope_at(size_t idx) {
        return scope_stack_[idx];
    }

    scope_stack_t& scopes() {
        return scope_stack_;
    }

    scope_type_stack_t& scope_types() {
        return scope_type_stack_;
    }



    bool is_in_global_scope() const noexcept {
        return top_scope_type() == ScopeType::global;
    }

    bool declare(const std::string& name) {
        if (top_scope().contains(name) && !is_in_global_scope()) {
            return false;
        }
        top_scope()[name] = ResolveState::declared;
        return true;
    }

    void define(const std::string& name) {
        assert(top_scope().find(name) != top_scope().end());
        assert(top_scope().find(name)->second == ResolveState::declared);
        top_scope()[name] = ResolveState::defined;
    }


    void set_depth(const Expr& expr, size_t depth) {
        depth_map_[&expr] = depth;
    }

    depth_map_t& depth_map() {
        return depth_map_;
    }


    bool is_in_function() const noexcept {
        return is_in_function_;
    }

};
