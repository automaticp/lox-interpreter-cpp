#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "Errors.hpp"

class ErrorReporter {
private:
    std::vector<ContextError> context_errs_;
    std::vector<ScannerError> scanner_errs_;

public:
    bool had_context_errors() const noexcept {
        return !context_errs_.empty();
    }

    bool had_scanner_errors() const noexcept {
        return !scanner_errs_.empty();
    }

    bool had_errors() const noexcept {
        return had_context_errors() || had_scanner_errors();
    }

    void scanner_error(ScannerError type, size_t line, std::string_view details) {
        scanner_errs_.push_back(type);
        report_scanner_error(type, line, details);
    }

    void context_error(ContextError type, std::string_view details) {
        context_errs_.push_back(type);
        report_context_error(type, details);
    }

    virtual ~ErrorReporter() = default;

protected:
    virtual void report_context_error(ContextError type, std::string_view details = "") = 0;

    virtual void report_scanner_error(ScannerError type, size_t line, std::string_view details = "") = 0;

};


class StreamErrorReporter : public ErrorReporter {
private:
    std::ostream& os_;
public:
    StreamErrorReporter(std::ostream& os) : os_{ os } {}

protected:
    void report_context_error(ContextError type, std::string_view details) override {
        os_ << "[Error @Context]:\n"
            << to_error_message(type);
        if (!details.empty()) {
            append_details(details);
        } else {
            os_ << '.' << std::endl;
        }
    }


    void report_scanner_error(ScannerError type, size_t line, std::string_view details) override {
        os_ << "[Error @Scanner] at line " << line << ":\n"
            << to_error_message(type);
        if (!details.empty()) {
            append_details(details);
        } else {
            os_ << '.' << std::endl;
        }
    }

private:
    void append_details(std::string_view details) {
        os_ << (details.size() < 16 ? ": " : ":\n    ");
        os_ << details << std::endl;
    }
};
