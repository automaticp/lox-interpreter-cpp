#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cxxopts.hpp>

#include "ErrorReporter.hpp"

void run(const std::string& text) {}

std::string read_file(const std::string& file) {

    std::ifstream fs{ file };

    if (!fs.fail()) {
        return {
            std::istreambuf_iterator<char>(fs),
            std::istreambuf_iterator<char>()
        };
    } else {
        throw std::runtime_error(
            "Cannot open source file: " + file
        );
    }
}

void run_file(const std::string& file) {
    run(read_file(file));
}

void run_prompt() {

    std::string line{};
    std::cout << "Starting lox interpreter prompt...\n";
    std::cout << "> ";
    while (std::getline(std::cin, line)) {
        run(line);
        std::cout << "> ";
    }
    std::cout << std::endl;

}



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
