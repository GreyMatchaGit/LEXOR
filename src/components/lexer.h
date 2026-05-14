#ifndef LEXER_H
#define LEXER_H

#include "../structs/tokens.h"
#include <string>
#include <vector>

class Lexer {
private:
    std::string input;
    int pos;
    int line;

    char peek(int offset = 0);  // Looks ahead at the next character(s) without consuming them.
    char advance();             // Consumes the next character and returns it.
    bool isAtEnd();             // Checks if we've reached the end of the input.
    
public:
    Lexer();
    std::vector<std::vector<Token>> lex(const std::string& source);
};

#endif
