#include "components/lexer.h"
#include "components/parser.h"
#include "components/evaluator.h"
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string filename = argv[1];
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Could not open file " << filename << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        try {
            Lexer lexer;
            auto tokens = lexer.lex(source);

            Parser parser(false);
            auto ast = parser.parse(tokens);

            Evaluator evaluator;
            evaluator.evaluate(ast.get());
        } catch (const std::exception& e) {
            std::cerr << "Runtime Error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "LEXOR interactive REPL. Type your statements." << std::endl;
        Lexer lexer;
        Parser parser(true);
        Evaluator evaluator;

        std::string line;
        while (true) {
            std::cout << ">> ";
            if (!std::getline(std::cin, line)) break;
            if (line == "exit" || line == "quit") break;
            
            try {
                auto tokens = lexer.lex(line);
                if (tokens.size() > 0 && !tokens.front().empty() && tokens.front().front().type != TokenType::END_OF_FILE) {
                     auto stmt = parser.parseSingleLine(tokens.front());
                     // Wrap inside a temp Program to evaluate
                     Program p;
                     p.statements.push_back(std::move(stmt));
                     evaluator.evaluate(&p);
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }

    return 0;
}