#include "tokens.h"

std::unordered_map<std::string, TokenType> keywords = {
    {"SCRIPT", TokenType::SCRIPT},
    {"AREA", TokenType::AREA},
    {"START", TokenType::START},
    {"END", TokenType::END},
    {"DECLARE", TokenType::DECLARE},
    {"INT", TokenType::INT_TYPE},
    {"FLOAT", TokenType::FLOAT_TYPE},
    {"CHAR", TokenType::CHAR_TYPE},
    {"BOOL", TokenType::BOOL_TYPE},
    {"PRINT", TokenType::PRINT},
    {"SCAN", TokenType::SCAN},
    {"IF", TokenType::IF},
    {"ELSE", TokenType::ELSE},
    {"FOR", TokenType::FOR},
    {"REPEAT", TokenType::REPEAT},
    {"WHEN", TokenType::WHEN},
    {"AND", TokenType::AND},
    {"OR", TokenType::OR},
    {"NOT", TokenType::NOT}
};

TokenType checkKeyword(const std::string& str) {
    if (keywords.find(str) != keywords.end()) {
        return keywords[str];
    }
    return TokenType::IDENTIFIER;
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::SCRIPT: return "SCRIPT";
        case TokenType::AREA: return "AREA";
        case TokenType::START: return "START";
        case TokenType::END: return "END";
        case TokenType::DECLARE: return "DECLARE";
        case TokenType::INT_TYPE: return "INT_TYPE";
        case TokenType::FLOAT_TYPE: return "FLOAT_TYPE";
        case TokenType::CHAR_TYPE: return "CHAR_TYPE";
        case TokenType::BOOL_TYPE: return "BOOL_TYPE";
        case TokenType::PRINT: return "PRINT";
        case TokenType::SCAN: return "SCAN";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::FOR: return "FOR";
        case TokenType::REPEAT: return "REPEAT";
        case TokenType::WHEN: return "WHEN";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::INT_LITERAL: return "INT_LITERAL";
        case TokenType::FLOAT_LITERAL: return "FLOAT_LITERAL";
        case TokenType::CHAR_LITERAL: return "CHAR_LITERAL";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        case TokenType::BOOL_LITERAL: return "BOOL_LITERAL";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::MODULO: return "MODULO";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::DOLLAR: return "DOLLAR";
        case TokenType::AMPERSAND: return "AMPERSAND";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::COLON: return "COLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
