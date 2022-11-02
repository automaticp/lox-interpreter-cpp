#pragma once
#include "ErrorReporter2.hpp"
#include <memory>
#include <utility>


// Simple interface that encapsulates error reporting.
// (Hides it really, as implementation detail)
// Otherwise many classes repeat the same redundant routine.
template<typename ErrorT>
class ErrorSender {
private:
    ErrorReporter& err_;

public:
    ErrorSender(ErrorReporter& err) : err_{ err } {}

    template<typename ...Args>
    void send_error(Args&&... args) const {
        err_.error(
            std::make_unique<ErrorT>(
                std::forward<Args>(args)...
            )
        );
    }

    ErrorReporter& error_reporter() const noexcept {
        return err_;
    }
};
