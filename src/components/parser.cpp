#include "parser.h"
#include <iostream>

Parser::Parser(bool isRepl) : replMode(isRepl), currentLine(0), currentToken(0) {}

bool Parser::isAtEnd() {
    return currentLine >= lines.size();
}

bool Parser::isLineAtEnd() {
    if (isAtEnd()) return true;
    return currentToken >= lines[currentLine].size();
}

Token Parser::peek() {
    while (!isLineAtEnd() && (lines[currentLine][currentToken].type == TokenType::INDENT || lines[currentLine][currentToken].type == TokenType::DEDENT)) {
        currentToken++;
    }
    if (isLineAtEnd()) {
        if (currentLine + 1 < lines.size()) return lines[currentLine + 1][0]; // fallback
        return {TokenType::END_OF_FILE, "", 0};
    }
    return lines[currentLine][currentToken];
}

Token Parser::advance() {
    Token t = peek();
    if (!isLineAtEnd()) currentToken++;
    return t;
}

Token Parser::previous() {
    int tempToken = currentToken - 1;
    while (tempToken >= 0 && (lines[currentLine][tempToken].type == TokenType::INDENT || lines[currentLine][tempToken].type == TokenType::DEDENT)) {
        tempToken--;
    }
    if (tempToken >= 0) return lines[currentLine][tempToken];
    return lines[currentLine][currentToken - 1]; // fallback
}

bool Parser::check(TokenType type) {
    if (isLineAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, std::string message) {
    if (check(type)) return advance();
    throw std::runtime_error("Line " + std::to_string(peek().line) + " Parser Error: " + message);
}

void Parser::advanceLine() {
    currentLine++;
    currentToken = 0;
}

std::unique_ptr<Program> Parser::parse(const std::vector<std::vector<Token>>& tokens) {
    lines = tokens;
    currentLine = 0;
    currentToken = 0;
    
    auto program = std::make_unique<Program>();

    if (!replMode) {
        // Enforce script boundaries
        bool foundArea = false;
        while (!isAtEnd()) {
            if (check(TokenType::SCRIPT) && lines[currentLine].size() > 1 && lines[currentLine][1].type == TokenType::AREA) {
                foundArea = true;
                advanceLine();
                break;
            }
            advanceLine();
        }
        if (!foundArea) throw std::runtime_error("Missing SCRIPT AREA");
        
        bool foundStart = false;
        while (!isAtEnd()) {
            bool ws = true;
            for (const auto& t : lines[currentLine]) {
                if (t.type != TokenType::INDENT && t.type != TokenType::DEDENT) ws = false;
            }
            if (lines[currentLine].empty() || ws) {
                advanceLine();
                continue;
            }
            
            if (lines[currentLine][0].type != TokenType::INDENT) {
                throw std::runtime_error("Line " + std::to_string(lines[currentLine][0].line) + " Parser Error: Must have indent after SCRIPT AREA");
            }
            
            // Check for START SCRIPT skipping the INDENT
            int tIndex = 1;
            while(tIndex < lines[currentLine].size() && (lines[currentLine][tIndex].type == TokenType::INDENT || lines[currentLine][tIndex].type == TokenType::DEDENT)) {
                tIndex++;
            }
            
            if (tIndex + 1 < lines[currentLine].size() && lines[currentLine][tIndex].type == TokenType::START && lines[currentLine][tIndex+1].type == TokenType::SCRIPT) {
                foundStart = true;
                advanceLine();
                break;
            } else {
                 throw std::runtime_error("Expected START SCRIPT after SCRIPT AREA");
            }
        }
        if (!foundStart) throw std::runtime_error("Missing START SCRIPT");
    }

    while (!isAtEnd()) {
        if (check(TokenType::END_OF_FILE)) break;
        if (check(TokenType::END) && lines[currentLine].size() > 1 && lines[currentLine][1].type == TokenType::SCRIPT) {
            break; // Valid exit
        }

        if (lines[currentLine].empty()) {
            advanceLine();
            continue;
        }
        
        // Skip purely indent/dedent lines if they somehow slip in at top level without context
        bool onlyWhitespace = true;
        for (const auto& t : lines[currentLine]) {
            if (t.type != TokenType::INDENT && t.type != TokenType::DEDENT) {
                onlyWhitespace = false;
                break;
            }
        }
        if (onlyWhitespace) {
            advanceLine(); continue;
        }

        program->statements.push_back(declaration());
        advanceLine();
    }
    
    return program;
}

std::unique_ptr<Statement> Parser::parseSingleLine(const std::vector<Token>& tokenLine) {
    lines = {tokenLine};
    currentLine = 0;
    currentToken = 0;
    return declaration();
}

std::unique_ptr<Statement> Parser::declaration() {
    if (match(TokenType::DECLARE)) {
        return varDecl();
    }
    return statement();
}

std::unique_ptr<Statement> Parser::varDecl() {
    TokenType type = advance().type;
    if (type != TokenType::INT_TYPE && type != TokenType::FLOAT_TYPE && type != TokenType::CHAR_TYPE && type != TokenType::BOOL_TYPE) {
        throw std::runtime_error("Expected INT, FLOAT, CHAR, or BOOL after DECLARE.");
    }

    auto declStmt = std::make_unique<VarDeclStatement>(type);

    do {
        Token name = consume(TokenType::IDENTIFIER, "Expected variable name.");
        std::unique_ptr<Expr> init = nullptr;
        if (match(TokenType::ASSIGN)) {
            init = expression();
        }
        VarDeclStatement::DeclInfo info{name.value, std::move(init)};
        declStmt->declarations.push_back(std::move(info));
    } while(match(TokenType::COMMA));

    return declStmt;
}

std::unique_ptr<Statement> Parser::statement() {
    if (match(TokenType::PRINT)) {
        consume(TokenType::COLON, "Expected ':' after PRINT.");
        return printStmt();
    }
    if (match(TokenType::SCAN)) {
        consume(TokenType::COLON, "Expected ':' after SCAN.");
        return scanStmt();
    }
    if (match(TokenType::IF)) return ifStmt();
    if (match(TokenType::FOR)) return forStmt();
    if (match(TokenType::REPEAT)) {
        if (match(TokenType::WHEN)) {
             return repeatStmt();
        }
        throw std::runtime_error("Expected WHEN after REPEAT.");
    }
    
    return assignStmt();
}

std::unique_ptr<Statement> Parser::assignStmt() {
    // We expect chained assignment target=target=expr
    std::vector<std::string> targets;
    targets.push_back(consume(TokenType::IDENTIFIER, "Expected variable name.").value);
    
    while(match(TokenType::ASSIGN)) {
        if (check(TokenType::IDENTIFIER) && currentToken + 1 < lines[currentLine].size() && lines[currentLine][currentToken + 1].type == TokenType::ASSIGN) {
            targets.push_back(advance().value);
        } else {
            auto value = expression();
            return std::make_unique<AssignStatement>(targets, std::move(value));
        }
    }
    throw std::runtime_error("Invalid assignment.");
}

std::unique_ptr<Statement> Parser::printStmt() {
    auto stmt = std::make_unique<PrintStatement>();
    do {
        stmt->expressions.push_back(expression());
    } while (match(TokenType::AMPERSAND));
    return stmt;
}

std::unique_ptr<Statement> Parser::scanStmt() {
    auto stmt = std::make_unique<ScanStatement>();
    do {
        stmt->targets.push_back(consume(TokenType::IDENTIFIER, "Expected variable name.").value);
    } while (match(TokenType::COMMA));
    return stmt;
}

std::unique_ptr<Statement> Parser::ifStmt() {
    consume(TokenType::LPAREN, "Expected '(' after IF.");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expected ')' after IF condition.");

    advanceLine(); // move to START IF line
    if (!check(TokenType::START) || lines[currentLine].size() < 2 || lines[currentLine][1].type != TokenType::IF) {
        throw std::runtime_error("Expected START IF");
    }
    
    auto thenBranch = block(); // Block will leave currentLine on END IF

    auto stmt = std::make_unique<IfStatement>(std::move(condition), std::move(thenBranch));

    // Handle ELSE IF / ELSE
    while (currentLine + 1 < lines.size()) {
        int nextL = currentLine + 1;
        // Skip purely whitespace lines
        while (nextL < lines.size()) {
            bool onlyWs = true;
            for (const auto& t : lines[nextL]) {
                if (t.type != TokenType::INDENT && t.type != TokenType::DEDENT) { onlyWs = false; break; }
            }
            if (onlyWs || lines[nextL].empty()) nextL++;
            else break;
        }

        if (nextL < lines.size() && lines[nextL][0].type == TokenType::ELSE) {
            currentLine = nextL; currentToken = 0; // Move from END IF to ELSE line
            
            advance(); // Consume ELSE
            if (match(TokenType::IF)) {
                consume(TokenType::LPAREN, "Expected '(' after ELSE IF.");
                auto elifCondition = expression();
                consume(TokenType::RPAREN, "Expected ')' after condition.");
                advanceLine();
                if (!check(TokenType::START) || lines[currentLine].size() < 2 || lines[currentLine][1].type != TokenType::IF) {
                    throw std::runtime_error("Expected START IF for ELSE IF");
                }
                auto elifBody = block();
                IfStatement::ElseIfBranch elifBranch{std::move(elifCondition), std::move(elifBody)};
                stmt->elseIfBranches.push_back(std::move(elifBranch));
            } else {
                // ELSE
                advanceLine();
                if (!check(TokenType::START) || lines[currentLine].size() < 2 || lines[currentLine][1].type != TokenType::IF) {
                    throw std::runtime_error("Expected START IF for ELSE");
                }
                stmt->elseBranch = block();
                break; // Only 1 ELSE allowed
            }
        } else {
            break;
        }
    }

    return stmt;
}

std::unique_ptr<Statement> Parser::forStmt() {
    consume(TokenType::LPAREN, "Expected '(' after FOR.");
    std::string initTarget = consume(TokenType::IDENTIFIER, "Expected init target.").value;
    consume(TokenType::ASSIGN, "Expected '='.");
    auto initVal = expression();
    consume(TokenType::COMMA, "Expected ','.");
    auto cond = expression();
    consume(TokenType::COMMA, "Expected ','.");
    std::string updateTarget = consume(TokenType::IDENTIFIER, "Expected update target.").value;
    consume(TokenType::ASSIGN, "Expected '='.");
    auto updateVal = expression();
    consume(TokenType::RPAREN, "Expected ')'.");
    
    advanceLine();
    if (!check(TokenType::START) || lines[currentLine].size() < 2 || lines[currentLine][1].type != TokenType::FOR) {
        throw std::runtime_error("Expected START FOR");
    }
    
    auto stmt = std::make_unique<ForStatement>();
    stmt->initTarget = initTarget;
    stmt->initValue = std::move(initVal);
    stmt->condition = std::move(cond);
    stmt->updateTarget = updateTarget;
    stmt->updateValue = std::move(updateVal);
    stmt->body = block(); // Block parses until END FOR (but wait, block specifically looks for "END X", so let's make block generic)

    return stmt;
}

std::unique_ptr<Statement> Parser::repeatStmt() {
    consume(TokenType::LPAREN, "Expected '(' for WHEN condition.");
    auto cond = expression();
    consume(TokenType::RPAREN, "Expected ')'.");
    
    advanceLine();
    if (!check(TokenType::START) || lines[currentLine].size() < 2 || lines[currentLine][1].type != TokenType::REPEAT) {
        throw std::runtime_error("Expected START REPEAT");
    }
    
    auto body = block();
    return std::make_unique<RepeatStatement>(std::move(cond), std::move(body));
}

std::vector<std::unique_ptr<Statement>> Parser::block() {
    std::vector<std::unique_ptr<Statement>> statements;
    advanceLine(); // move past START block keyword line
    
    while (!isAtEnd()) {
        if (lines[currentLine].empty() || check(TokenType::INDENT) || check(TokenType::DEDENT)) {
            advanceLine();
            continue;
        }
        
        if (check(TokenType::END)) {
            // Found END block
            break;
        }

        statements.push_back(declaration());
        advanceLine();
    }
    
    // consume END X line (where X is IF, FOR, REPEAT)
    // DO NOT invoke advanceLine() here, leave it to the outer declaration loop!
    return statements;
}

std::unique_ptr<Expr> Parser::expression() {
    return logicalOr();
}

std::unique_ptr<Expr> Parser::logicalOr() {
    auto expr = logicalAnd();
    while (match(TokenType::OR)) {
        Token op = previous();
        auto right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
    auto expr = equality();
    while (match(TokenType::AND)) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    auto expr = comparison();
    while (match(TokenType::EQUAL_EQUAL) || match(TokenType::NOT_EQUAL)) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    auto expr = term();
    while (match(TokenType::GREATER) || match(TokenType::GREATER_EQUAL) || match(TokenType::LESS) || match(TokenType::LESS_EQUAL)) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    auto expr = factor();
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    auto expr = unary();
    while (match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::MODULO)) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match(TokenType::NOT) || match(TokenType::MINUS) || match(TokenType::PLUS)) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary() {
    if (match(TokenType::INT_LITERAL) || match(TokenType::FLOAT_LITERAL) || match(TokenType::CHAR_LITERAL) || match(TokenType::BOOL_LITERAL) || match(TokenType::STRING_LITERAL) || match(TokenType::DOLLAR)) {
        return std::make_unique<LiteralExpr>(previous());
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<IdentifierExpr>(previous().value);
    }

    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expected ')' after expression.");
        return expr;
    }

    throw std::runtime_error("Expected expression.");
}
