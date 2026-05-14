#include "lexer.h"
#include <cctype>
#include <sstream>
#include <iostream>

Lexer::Lexer() {
}

char Lexer::peek(int offset) {
    if (pos + offset >= input.length()) return '\0';
    return input[pos + offset];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    return input[pos++];
}

bool Lexer::isAtEnd() {
    return pos >= input.length();
}


std::vector<std::vector<Token>> Lexer::lex(const std::string& source) {
    input = source;
    pos = 0;
    line = 1;
    
    std::vector<std::vector<Token>> tokensByLine;
    std::vector<Token> currentLineTokens;

    while (!isAtEnd()) {
        // Skip leading whitespace
        while (peek() == ' ' || peek() == '\t') {
            advance();
        }
        
        // Empty line handling
        if (peek() == '\n' || peek() == '\r') {
            if (peek() == '\r' && peek(1) == '\n') advance();
            advance();
            tokensByLine.push_back({{TokenType::ERROR, "Empty lines are not allowed.", line}});
            line++;
            continue;
        }   

        // Comment handling
        if (peek() == '%' && peek(1) == '%') {
            while (!isAtEnd() && peek() != '\n' && peek() != '\r') advance();
            if (!isAtEnd() && (peek() == '\n' || peek() == '\r')) {
                if (peek() == '\r' && peek(1) == '\n') advance();
                advance();
                line++;
            }
            continue;
        }
        
        if (isAtEnd()) break;
        
        // Read tokens for this line
        while (!isAtEnd() && peek() != '\n' && peek() != '\r') {
            char c = peek();
            
            // Skip whitespace within the line
            if (c == ' ' || c == '\t') {
                advance(); 
                continue;
            }
            
            // Handles inline comments
            if (c == '%' && peek(1) == '%') {
                while (!isAtEnd() && peek() != '\n') advance();
                break;
            }
            
            // Identifiers and Keywords
            if (isalpha(c) || c == '_') {
                std::string ident = "";
                while (isalnum(peek()) || peek() == '_') {
                    ident += advance();
                }
                currentLineTokens.push_back({checkKeyword(ident), ident, line});
            } else if (isdigit(c)) {
                std::string num = "";
                bool isFloat = false;
                bool isValid = true;

                // 3.14.2
                while (isdigit(peek()) || peek() == '.') {
                    if (peek() == '.') {
                        if (isFloat) { // Ensure only one dot for floats
                            isValid = false;
                        }
                        isFloat = true;
                    }
                    num += advance();
                }

                if (!isValid || num == "." || num.front() == '.' || num.back() == '.') {
                    currentLineTokens.push_back({TokenType::ERROR, "Invalid number format: " + num, line});
                } else {
                    currentLineTokens.push_back({isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, num, line});
                }
            } else if (c == '"') { 
                advance(); // skip "
                std::string str = "";
                while (!isAtEnd() && peek() != '"') {
                    str += advance();
                }
                if (!isAtEnd()) advance(); // skip "
                currentLineTokens.push_back({TokenType::STRING_LITERAL, str, line});
            } else if (c == '\'') {
                advance(); // skip '
                std::string ch = "";
                if (!isAtEnd() && peek() != '\'') {
                    ch += advance();
                }
                if (!isAtEnd()) advance(); // skip '
                currentLineTokens.push_back({TokenType::CHAR_LITERAL, ch, line});
            } else if (c == '[') {
                advance(); // skip [
                std::string ch = "";
                if (!isAtEnd()) {
                    ch += advance();
                }
                if (!isAtEnd() && peek() == ']') {
                    advance(); // skip closing ]
                }
                currentLineTokens.push_back({TokenType::CHAR_LITERAL, ch, line});
            } else {
                c = advance();
                switch(c) {
                    case '+': currentLineTokens.push_back({TokenType::PLUS, "+", line}); break;
                    case '-': currentLineTokens.push_back({TokenType::MINUS, "-", line}); break;
                    case '*': currentLineTokens.push_back({TokenType::STAR, "*", line}); break;
                    case '/': currentLineTokens.push_back({TokenType::SLASH, "/", line}); break;
                    case '%': currentLineTokens.push_back({TokenType::MODULO, "%", line}); break;
                    case '(': currentLineTokens.push_back({TokenType::LPAREN, "(", line}); break;
                    case ')': currentLineTokens.push_back({TokenType::RPAREN, ")", line}); break;
                    case ':': currentLineTokens.push_back({TokenType::COLON, ":", line}); break;
                    case ',': currentLineTokens.push_back({TokenType::COMMA, ",", line}); break;
                    case '$': currentLineTokens.push_back({TokenType::DOLLAR, "$", line}); break;
                    case '&': currentLineTokens.push_back({TokenType::AMPERSAND, "&", line}); break;
                    case '=':
                        if (peek() == '=') {
                            advance();
                            currentLineTokens.push_back({TokenType::EQUAL_EQUAL, "==", line});
                        } else {
                            currentLineTokens.push_back({TokenType::ASSIGN, "=", line});
                        }
                        break;
                    case '<':
                        if (peek() == '>') {
                            advance();
                            currentLineTokens.push_back({TokenType::NOT_EQUAL, "<>", line});
                        } else if (peek() == '=') {
                            advance();
                            currentLineTokens.push_back({TokenType::LESS_EQUAL, "<=", line});
                        } else {
                            currentLineTokens.push_back({TokenType::LESS, "<", line});
                        }
                        break;
                    case '>':
                        if (peek() == '=') {
                            advance();
                            currentLineTokens.push_back({TokenType::GREATER_EQUAL, ">=", line});
                        } else {
                            currentLineTokens.push_back({TokenType::GREATER, ">", line});
                        }
                        break;
                    default:
                        currentLineTokens.push_back({TokenType::ERROR, std::string(1, c), line});
                        break;
                }
            }
        }
        
        if (!currentLineTokens.empty()) {
            tokensByLine.push_back(currentLineTokens);
            currentLineTokens.clear();
        }

        if (!isAtEnd() && (peek() == '\n' || peek() == '\r')) {
            if (peek() == '\r' && peek(1) == '\n') advance();
            advance();
            line++;
        }
    }
    
    // EOF token
    tokensByLine.push_back({{TokenType::END_OF_FILE, "EOF", line}});

    return tokensByLine;
}
