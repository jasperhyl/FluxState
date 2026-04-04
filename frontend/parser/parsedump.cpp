#include "../lexer/lexer.hpp"
#include "parser.hpp"
#include "error.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define _isatty isatty
#define _fileno fileno
#endif

static std::string ReadAllFromIstream(std::istream& in) {
    std::ostringstream oss;
    oss << in.rdbuf();
    return oss.str();
}

static std::string ReadFileToString(const std::string& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("failed to open file: " + path);
    }
    return ReadAllFromIstream(file);
}

int main(int argc, char** argv) {
    try {
        std::string source;

        if (argc >= 2) {
            source = ReadFileToString(argv[1]);
        } else {
            bool stdin_is_tty = _isatty(_fileno(stdin)) != 0;
            if (!stdin_is_tty) {
                source = ReadAllFromIstream(std::cin);
            }
            if (source.empty()) {
                source = R"(
machine AccessControl {
    state Locked, Unlocked;
    initial Locked;

    event CardScanned(string who);

    transition from Locked on CardScanned(who) when true -> Unlocked do { };
    transition from Unlocked after 10s when true -> Locked do { };
}
)";
            }
        }

        flux::Lexer lexer(std::move(source));
        auto tokens = lexer.Tokenize();

        flux::Parser parser(std::move(tokens));
        auto program = parser.ParseProgram();

        std::cout << "Parsing succeeded. Found " << program.machines.size() << " machine(s).\n";
        for (const auto& mach : program.machines) {
            std::cout << "Machine: " << mach.name << "\n";
            std::cout << "  States: ";
            for (const auto& s : mach.states) std::cout << s << " ";
            std::cout << "\n  Initial: " << mach.initial_state << "\n";
            std::cout << "  Events: " << mach.events.size() << "\n";
            std::cout << "  Transitions: " << mach.transitions.size() << "\n";
        }
        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}