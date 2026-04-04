// main.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "lexer/lexer.hpp"
#include "parser.hpp"

using namespace flux;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file.flux>" << std::endl;
        return 1;
    }

    // 读取源文件
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open file " << argv[1] << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 词法分析
    Lexer lexer(source);
    std::vector<Token> tokens;
    try {
        tokens = lexer.Tokenize();
    } catch (const std::runtime_error& e) {
        std::cerr << "Lexical error: " << e.what() << std::endl;
        return 1;
    }

    // 可选：打印 tokens（调试用）
    std::cout << "Tokens:" << std::endl;
    for (const auto& tok : tokens) {
        std::cout << "  " << TokenTypeToString(tok.type) << " \"" << tok.lexeme << "\" at "
                  << tok.line << ":" << tok.column << std::endl;
    }

    // 语法分析
    Parser parser(std::move(tokens));
    try {
        Program program = parser.ParseProgram();
        std::cout << "\nParsing succeeded. Found " << program.machines.size() << " machine(s)." << std::endl;
        for (const auto& m : program.machines) {
            std::cout << "Machine: " << m.name << std::endl;
            std::cout << "  States: ";
            for (const auto& s : m.states) std::cout << s << " ";
            std::cout << "\n  Initial state: " << m.initial_state << std::endl;
            std::cout << "  Events: ";
            for (const auto& e : m.events) {
                std::cout << e.name << "(";
                for (size_t i = 0; i < e.params.size(); ++i) {
                    if (i > 0) std::cout << ",";
                    std::cout << ParamTypeToString(e.params[i].type) << " " << e.params[i].name;
                }
                std::cout << ") ";
            }
            std::cout << "\n  Transitions: " << m.transitions.size() << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}