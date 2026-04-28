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
    std::vector<int> indentStack;

    char peek(int offset = 0);
    char advance();
    bool isAtEnd();
    
public:
    Lexer();
    std::vector<std::vector<Token>> lex(const std::string& source);
};

#endif
