#pragma once
#include <utility>
#include <string>



// The only thing that statically defines the set of all
// possible error types. Could be removed, maybe, along with
// the category() method, and replaced with typeid comparisons
// for infinite extensibility.
enum class ErrorCategory {
    context,
    scanner,
    parser,
    import,
    resolver,
    interpreter,
    other
};






// Implementing this interface enables you to report the errors
// through ErrorReporter.

class IError {
public:
    virtual ErrorCategory category() const = 0;
    virtual std::string message() const = 0;

    IError() = default;
    IError(const IError&) = default;
    IError(IError&&) = default;
    IError& operator=(const IError&) = default;
    IError& operator=(IError&&) = default;
    virtual ~IError() = default;
};




// Dummy error that has no structural information.
// Can be used temporarily during development.
// Replace later with specialized error class
// or I'll call the Design Police on you.
class SimpleError : public IError {
public:
    std::string msg;

    SimpleError(std::string msg) :
        msg{ std::move(msg) } {}

    ErrorCategory category() const override {
        return ErrorCategory::other;
    }

    std::string message() const override {
        return msg;
    }
};
