#ifndef AST_H
#define AST_H

#include "tokens.h"
#include <vector>
#include <string>
#include <memory>

class Program;
class VarDeclStatement;
class AssignStatement;
class PrintStatement;
class ScanStatement;
class IfStatement;
class ForStatement;
class RepeatStatement;
class BinaryExpr;
class UnaryExpr;
class LiteralExpr;
class IdentifierExpr;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(Program* node) = 0;
    virtual void visit(VarDeclStatement* node) = 0;
    virtual void visit(AssignStatement* node) = 0;
    virtual void visit(PrintStatement* node) = 0;
    virtual void visit(ScanStatement* node) = 0;
    virtual void visit(IfStatement* node) = 0;
    virtual void visit(ForStatement* node) = 0;
    virtual void visit(RepeatStatement* node) = 0;
    virtual void visit(BinaryExpr* node) = 0;
    virtual void visit(UnaryExpr* node) = 0;
    virtual void visit(LiteralExpr* node) = 0;
    virtual void visit(IdentifierExpr* node) = 0;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor* visitor) = 0;
};

class Statement : public ASTNode {};
class Expr : public ASTNode {};

class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// ---------- Expressions ----------

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    
    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r) : left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token o, std::unique_ptr<Expr> r) : op(std::move(o)), right(std::move(r)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class LiteralExpr : public Expr {
public:
    Token value;
    LiteralExpr(Token v) : value(std::move(v)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class IdentifierExpr : public Expr {
public:
    std::string name;
    IdentifierExpr(std::string n) : name(std::move(n)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

// ---------- Statements ----------

class VarDeclStatement : public Statement {
public:
    TokenType dataType;
    struct DeclInfo {
        std::string name;
        std::unique_ptr<Expr> initializer; // can be null
    };
    std::vector<DeclInfo> declarations;
    
    VarDeclStatement(TokenType t) : dataType(t) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class AssignStatement : public Statement {
public:
    std::vector<std::string> targets; // For chained assignment e.g. x=y=4
    std::unique_ptr<Expr> value;
    
    AssignStatement(std::vector<std::string> t, std::unique_ptr<Expr> v) : targets(std::move(t)), value(std::move(v)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class PrintStatement : public Statement {
public:
    std::vector<std::unique_ptr<Expr>> expressions; // list of expressions separated by &
    PrintStatement() {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class ScanStatement : public Statement {
public:
    std::vector<std::string> targets; // variable names
    ScanStatement() {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class IfStatement : public Statement {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Statement>> thenBranch;
    
    struct ElseIfBranch {
        std::unique_ptr<Expr> condition;
        std::vector<std::unique_ptr<Statement>> body;
    };
    std::vector<ElseIfBranch> elseIfBranches;
    
    std::vector<std::unique_ptr<Statement>> elseBranch; // empty means no else

    IfStatement(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Statement>> thenB) 
        : condition(std::move(cond)), thenBranch(std::move(thenB)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class ForStatement : public Statement {
public:
    std::string initTarget;
    std::unique_ptr<Expr> initValue;

    std::unique_ptr<Expr> condition;

    std::string updateTarget;
    std::unique_ptr<Expr> updateValue;

    std::vector<std::unique_ptr<Statement>> body;

    ForStatement() {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

class RepeatStatement : public Statement {
public:
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Statement>> body;

    RepeatStatement(std::unique_ptr<Expr> cond, std::vector<std::unique_ptr<Statement>> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void accept(ASTVisitor* visitor) override { visitor->visit(this); }
};

#endif
