#include <iostream>
#include <string>
#include <cxxopts.hpp>


void run_file(const std::string& file) {}
void run_prompt() {}


int main(int argc, char* argv[]) {

    cxxopts::Options opts{ "lox-twi", "lox tree-walk interpreter" };
    opts.add_options()
        ("h,help", "Show help and exit")
        ("input", "Input file to be parsed", cxxopts::value<std::string>());
    opts.parse_positional("input");
    opts.positional_help("file");

    auto optres = opts.parse(argc, argv);

    if (optres.count("help")) {
        std::cout << opts.help() << '\n';
        return 0;
    }

    if (optres.count("input")) {
        run_file(optres["input"].as<std::string>());
    } else {
        run_prompt();
    }

    return 0;
}
