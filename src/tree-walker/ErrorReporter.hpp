#pragma once
#include <iostream>
#include <string>
#include <string_view>

class ErrorReporter {
private:
    bool had_error_{ false };
public:
    void reset() noexcept {
        had_error_ = false;
    }

    bool had_error() const noexcept { return had_error_; }

    void error(size_t line, std::string_view message, std::string_view hint = "") noexcept {
        report(line, message, hint);
        had_error_ = true;
    }

    virtual ~ErrorReporter() = default;

protected:
    virtual void report(size_t line, std::string_view message, std::string_view hint) = 0;

};


class StreamErrorReporter : public ErrorReporter {
private:
    std::ostream& os_;
public:
    StreamErrorReporter(std::ostream& os) : os_{ os } {}

protected:
    void report(size_t line, std::string_view message, std::string_view hint) override {
        os_ << "[Error] at line " << line << ": "
            << hint << '\n' << message << std::endl;
    }
};
