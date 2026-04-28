#include "lexer.h"
#include <cctype>
#include <sstream>
#include <iostream>

Lexer::Lexer() {
    indentStack.push_back(0);
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
    indentStack.clear();
    indentStack.push_back(0);

    while (!isAtEnd()) {
        int currentIndent = 0;
        
        // Count indentation
        while (peek() == ' ' || peek() == '\t') {
            if (peek() == '\t') currentIndent += 4;
            else currentIndent += 1;
            advance();
        }
        
        // Skip empty lines or comment-only lines
        if (peek() == '\n' || peek() == '\r') {
            if (peek() == '\r' && peek(1) == '\n') advance();
            advance();
            line++;
            continue;
        }

        if (peek() == '%' && peek(1) == '%') {
            while (!isAtEnd() && peek() != '\n') advance();
            continue;
        }
        
        if (isAtEnd()) break;

        // Process Indentation logic before reading line tokens
        if (currentIndent > indentStack.back()) {
            indentStack.push_back(currentIndent);
            currentLineTokens.push_back({TokenType::INDENT, "INDENT", line});
        } else if (currentIndent < indentStack.back()) {
            while (indentStack.size() > 1 && currentIndent < indentStack.back()) {
                indentStack.pop_back();
                currentLineTokens.push_back({TokenType::DEDENT, "DEDENT", line});
            }
        }
        
        // Read tokens for this line
        while (!isAtEnd() && peek() != '\n' && peek() != '\r') {
            char c = peek();
            
            if (c == ' ' || c == '\t') {
                advance(); continue;
            }

            if (c == '%' && peek(1) == '%') {
                while (!isAtEnd() && peek() != '\n') advance();
                break;
            }

            if (isalpha(c) || c == '_') {
                std::string ident = "";
                while (isalnum(peek()) || peek() == '_') {
                    ident += advance();
                }
                currentLineTokens.push_back({checkKeyword(ident), ident, line});
            } else if (isdigit(c)) {
                std::string num = "";
                bool isFloat = false;
                while (isdigit(peek()) || peek() == '.') {
                    if (peek() == '.') isFloat = true;
                    num += advance();
                }
                currentLineTokens.push_back({isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL, num, line});
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
    
    // dedent all remaining
    currentLineTokens.clear();
    while (indentStack.size() > 1) {
        indentStack.pop_back();
        currentLineTokens.push_back({TokenType::DEDENT, "DEDENT", line});
    }
    if (!currentLineTokens.empty()) tokensByLine.push_back(currentLineTokens);
    
    // EOF token
    tokensByLine.push_back({{TokenType::END_OF_FILE, "EOF", line}});

    return tokensByLine;
}
