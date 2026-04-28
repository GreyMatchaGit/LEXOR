#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../structs/ast.h"
#include "../structs/environment.h"
#include <memory>
#include <vector>

class Evaluator : public ASTVisitor {
private:
    Environment* env;
    std::vector<std::unique_ptr<Environment>> envStack;
    
    RuntimeValue currentResult; 

    void pushEnv();
    void popEnv();

    RuntimeValue evaluateExpr(Expr* expr);
    
public:
    Evaluator();
    ~Evaluator() override = default;

    void evaluate(Program* program);

    void visit(Program* node) override;
    void visit(VarDeclStatement* node) override;
    void visit(AssignStatement* node) override;
    void visit(PrintStatement* node) override;
    void visit(ScanStatement* node) override;
    void visit(IfStatement* node) override;
    void visit(ForStatement* node) override;
    void visit(RepeatStatement* node) override;
    void visit(BinaryExpr* node) override;
    void visit(UnaryExpr* node) override;
    void visit(LiteralExpr* node) override;
    void visit(IdentifierExpr* node) override;
};

#endif
