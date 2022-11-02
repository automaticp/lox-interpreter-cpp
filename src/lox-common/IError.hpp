#pragma once
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




