#pragma once
#include "Token.hpp"
#include "IError.hpp"
#include <boost/unordered_map.hpp>
#include <fmt/core.h>
#include <string_view>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <cassert>
#include <iostream>
#include <algorithm>





class ErrorReporter {
private:
    std::vector<std::unique_ptr<IError>> errors_;

public:
    ErrorReporter() = default;

    void reset() {
        errors_.clear();
    }

    void error(std::unique_ptr<IError> error) {
        assert(error);
        report(*error);
        errors_.emplace_back(std::move(error));
    };

    bool had_errors_of_category(const ErrorCategory& category) const noexcept {
        return std::any_of(
            errors_.begin(), errors_.end(),
            [&category](const auto& err) {
                return err->category() == category;
            }
        );
    }

    bool had_errors() const noexcept {
        return !errors_.empty();
    }

    ErrorReporter(const ErrorReporter&) = delete;
    ErrorReporter& operator=(const ErrorReporter&) = delete;
    ErrorReporter(ErrorReporter&&) = default;
    ErrorReporter& operator=(ErrorReporter&&) = default;
    virtual ~ErrorReporter() = default;
protected:
    virtual void report(const IError& err) = 0;

};






class StreamErrorReporter : public ErrorReporter {
private:
    std::ostream& os_;

public:
    StreamErrorReporter(std::ostream& os) : os_{ os } {}

protected:
    void report(const IError& err) override {
        os_ << err.message();
    }
};



