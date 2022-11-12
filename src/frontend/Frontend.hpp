#pragma once
#include "ErrorReporter.hpp"
#include "Scanner.hpp"
#include "Importer.hpp"
#include "Parser.hpp"
#include "Resolver.hpp"



class Frontend {
private:
    Importer importer_;
    Parser parser_;
    Resolver resolver_;

public:
    explicit Frontend(ErrorReporter& err) :
        importer_{ err },
        parser_{ err },
        resolver_{ err }
    {}

    Importer& importer() noexcept { return importer_; }
    Parser& parser() noexcept { return parser_; }
    Resolver& resolver() noexcept { return resolver_; }

// Put code here
};

