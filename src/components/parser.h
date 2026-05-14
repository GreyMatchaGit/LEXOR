#ifndef PARSER_H
#define PARSER_H

#include "../structs/ast.h"
#include "../structs/tokens.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
private:
    std::vector<std::vector<Token>> lines;
    int currentLine;
    int currentToken;
    bool replMode;

    bool isAtEnd();
    bool isLineAtEnd();
    Token peek();
    Token advance();
    Token previous();
    bool match(TokenType type);
    bool check(TokenType type);
    Token consume(TokenType type, std::string message);

    void advanceLine();
    
    // Parsing helpers
    std::unique_ptr<Statement> declaration();
    std::unique_ptr<Statement> statement();
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> logicalOr();
    std::unique_ptr<Expr> logicalAnd();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();
    
    std::unique_ptr<Statement> varDecl();
    std::unique_ptr<Statement> assignStmt(); // handles target = expr or x = y = 4
    std::unique_ptr<Statement> printStmt();
    std::unique_ptr<Statement> scanStmt();
    std::unique_ptr<Statement> ifStmt();
    std::unique_ptr<Statement> forStmt();
    std::unique_ptr<Statement> repeatStmt();

    std::vector<std::unique_ptr<Statement>> block();

public:
    Parser(bool isRepl = false);
    std::unique_ptr<Program> parse(const std::vector<std::vector<Token>>& tokens);
    std::unique_ptr<Statement> parseSingleLine(const std::vector<Token>& tokenLine);
};

#endif
