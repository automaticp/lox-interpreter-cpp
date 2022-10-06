#pragma once
#include <iostream>
#include <string>


class ErrorReporter {
private:
    bool had_error_{ false };
public:
    void reset() noexcept {
        had_error_ = false;
    }

    bool had_error() const noexcept { return had_error_; }

    void error(size_t line, const std::string& message, const std::string& hint) noexcept {
        report(line, message, hint);
        had_error_ = true;
    }

    virtual ~ErrorReporter() = default;

protected:
    virtual void report(size_t line, const std::string& message, const std::string& hint) = 0;

};


class StreamErrorReporter : public ErrorReporter {
private:
    std::ostream& os_;
public:
    StreamErrorReporter(std::ostream& os) : os_{ os } {}

protected:
    void report(size_t line, const std::string& message, const std::string& hint) override {
        os_ << "[Error] at line " << line << ": "
            << hint << '\n' << message << std::endl;
    }
};
