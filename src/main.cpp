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
        std::cout << "LEXOR interactive REPL. Type your script. Type 'END SCRIPT' to execute, or 'exit' to quit." << std::endl;
        
        std::string source = "";
        std::string line;
        while (true) {
            std::cout << ">> ";
            if (!std::getline(std::cin, line)) break;
            if (line == "exit" || line == "quit") break;
            
            source += line + "\n";
            
            // Trim whitespace to check for END SCRIPT
            std::string trimmed = line;
            size_t startpos = trimmed.find_first_not_of(" \t\r\n");
            if (startpos != std::string::npos) {
                trimmed = trimmed.substr(startpos);
            } else {
                trimmed = "";
            }
            
            size_t endpos = trimmed.find_last_not_of(" \t\r\n");
            if (endpos != std::string::npos) {
                trimmed = trimmed.substr(0, endpos + 1);
            }

            if (trimmed == "END SCRIPT") {
                try {
                    Lexer lexer;
                    auto tokens = lexer.lex(source);
                    Parser parser(false); // Evaluate full script mode
                    auto ast = parser.parse(tokens);
                    Evaluator evaluator;
                    evaluator.evaluate(ast.get());
                    std::cout << std::endl; // newline to prevent output concatenation
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << std::endl;
                }
                source = ""; // Reset for next script
            }
        }
    }

    return 0;
}