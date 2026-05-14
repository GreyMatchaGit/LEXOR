#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <vector>
#include <unordered_map>

enum class TokenType {
    // Keywords
    SCRIPT, AREA, START, END, DECLARE,
    INT_TYPE, FLOAT_TYPE, CHAR_TYPE, BOOL_TYPE,
    PRINT, SCAN,
    IF, ELSE, FOR, REPEAT, WHEN,
    AND, OR, NOT,

    // Values / Identifiers
    IDENTIFIER,
    INT_LITERAL, FLOAT_LITERAL,
    CHAR_LITERAL, STRING_LITERAL, BOOL_LITERAL,

    // Operators
    PLUS, MINUS, STAR, SLASH, MODULO,
    GREATER, LESS, GREATER_EQUAL, LESS_EQUAL, EQUAL_EQUAL, NOT_EQUAL,
    ASSIGN,

    // Symbols & Specical
    DOLLAR, AMPERSAND, LBRACKET, RBRACKET, // $, &, [, ]
    COLON, COMMA, LPAREN, RPAREN,

    // Whitespace / Scope

    // End of File
    END_OF_FILE,
    ERROR
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

// Helper function to map string to keyword (if applicable)
TokenType checkKeyword(const std::string& str);
std::string tokenTypeToString(TokenType type);

#endif
