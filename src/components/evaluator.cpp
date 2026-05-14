#include "evaluator.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

Evaluator::Evaluator() {
    auto initialEnv = std::make_unique<Environment>();
    envStack.push_back(std::move(initialEnv));
    env = envStack.back().get();
}

void Evaluator::pushEnv() {
    auto newEnv = std::make_unique<Environment>(env);
    envStack.push_back(std::move(newEnv));
    env = envStack.back().get();
}

void Evaluator::popEnv() {
    if (envStack.size() > 1) {
        envStack.pop_back();
        env = envStack.back().get();
    }
}

RuntimeValue Evaluator::evaluateExpr(Expr* expr) {
    if (expr) {
        expr->accept(this);
    }
    return currentResult;
}

void Evaluator::evaluate(Program* program) {
    program->accept(this);
}

void Evaluator::visit(Program* node) {
    for (auto& stmt : node->statements) {
        stmt->accept(this);
    }
}

void Evaluator::visit(VarDeclStatement* node) {
    for (auto& decl : node->declarations) {
        RuntimeValue val;
        
        switch (node->dataType) {
            case TokenType::INT_TYPE: val = RuntimeValue(0); break;
            case TokenType::FLOAT_TYPE: val = RuntimeValue(0.0f); break;
            case TokenType::CHAR_TYPE: val = RuntimeValue('\0'); break;
            case TokenType::BOOL_TYPE: val = RuntimeValue(false); break;
            default: break;
        }

        if (decl.initializer) {
            RuntimeValue initVal = evaluateExpr(decl.initializer.get());
            if (val.type == DataType::INT && initVal.type == DataType::INT) val.value = initVal.value;
            else if (val.type == DataType::FLOAT && initVal.type == DataType::FLOAT) val.value = initVal.value;
            else if (val.type == DataType::CHAR && initVal.type == DataType::CHAR) val.value = initVal.value;
            else if (val.type == DataType::BOOL && initVal.type == DataType::BOOL) val.value = initVal.value;
            else if (val.type == DataType::BOOL && initVal.type == DataType::STRING) {
                if (std::get<std::string>(initVal.value) == "TRUE") val.value = true;
                else if (std::get<std::string>(initVal.value) == "FALSE") val.value = false;
                else throw std::runtime_error("Invalid BOOL value. Expected \"TRUE\" or \"FALSE\".");
            }
            else throw std::runtime_error("Type mismatch on initialization.");
        }
        
        env->declareVariable(decl.name, val);
    }
}

void Evaluator::visit(AssignStatement* node) {
    RuntimeValue val = evaluateExpr(node->value.get());
    for (const auto& target : node->targets) {
        RuntimeValue targetVar = env->getVariable(target);
        

        if (targetVar.type == DataType::BOOL && val.type == DataType::STRING) {
             if (std::get<std::string>(val.value) == "TRUE") targetVar.value = true;
             else if (std::get<std::string>(val.value) == "FALSE") targetVar.value = false;
             else throw std::runtime_error("Invalid BOOL string. Expected \"TRUE\" or \"FALSE\".");
             env->assignVariable(target, targetVar);
             continue;
        }
        
        env->assignVariable(target, val);
    }
}

std::string stringify(const RuntimeValue& v) {
    switch (v.type) {
        case DataType::INT: return std::to_string(std::get<int>(v.value));
        case DataType::FLOAT: return std::to_string(std::get<float>(v.value));
        case DataType::CHAR: return std::string(1, std::get<char>(v.value));
        case DataType::BOOL: return std::get<bool>(v.value) ? "TRUE" : "FALSE";
        case DataType::STRING: return std::get<std::string>(v.value);
    }
    return "";
}

void Evaluator::visit(PrintStatement* node) {
    std::string out = "";
    for (size_t i = 0; i < node->expressions.size(); ++i) {
        RuntimeValue val = evaluateExpr(node->expressions[i].get());
        if (val.type == DataType::STRING && std::get<std::string>(val.value) == "$") {
             out += "\n";
        } else {
             out += stringify(val);
        }
    }
    std::cout << out;
}

void Evaluator::visit(ScanStatement* node) {
    for (size_t i = 0; i < node->targets.size(); ++i) {
        if (i > 0) {
            char comma;
            std::cin >> comma;
            if (comma != ',') {
                throw std::runtime_error("Expected ',' between input values for SCAN.");
            }
        }
        const auto& target = node->targets[i];
        RuntimeValue v = env->getVariable(target);
        std::string s;
        std::cin >> s;

        if (v.type == DataType::INT) {
            if (s.find('.') != std::string::npos) {
                throw std::runtime_error("Type mismatch: Expected INT, got FLOAT input for '" + target + "'.");
            }
            try {
                size_t pos;
                int x = std::stoi(s, &pos);
                if (pos != s.length()) throw std::runtime_error("Invalid INT input for '" + target + "'.");
                v.value = x;
            } catch (...) {
                throw std::runtime_error("Invalid INT input for '" + target + "'.");
            }
        } else if (v.type == DataType::FLOAT) {
            try {
                size_t pos;
                float x = std::stof(s, &pos);
                if (pos != s.length()) throw std::runtime_error("Invalid FLOAT input for '" + target + "'.");
                v.value = x;
            } catch (...) {
                throw std::runtime_error("Invalid FLOAT input for '" + target + "'.");
            }
        } else if (v.type == DataType::CHAR) {
            if (s.length() != 1) throw std::runtime_error("Invalid CHAR input for '" + target + "'.");
            v.value = s[0];
        } else if (v.type == DataType::BOOL) {
            if (s == "TRUE") v.value = true;
            else if (s == "FALSE") v.value = false;
            else throw std::runtime_error("Invalid BOOL input for '" + target + "'. Expected TRUE or FALSE.");
        }
        env->assignVariable(target, v);
    }
}

void Evaluator::visit(IfStatement* node) {
    RuntimeValue condVal = evaluateExpr(node->condition.get());
    if (condVal.type != DataType::BOOL) throw std::runtime_error("IF condition must be BOOL.");
    
    if (std::get<bool>(condVal.value)) {
        pushEnv();
        for (auto& stmt : node->thenBranch) stmt->accept(this);
        popEnv();
        return;
    }
    
    for (auto& elif : node->elseIfBranches) {
        RuntimeValue elifCond = evaluateExpr(elif.condition.get());
        if (elifCond.type != DataType::BOOL) throw std::runtime_error("ELSE IF condition must be BOOL.");
        if (std::get<bool>(elifCond.value)) {
            pushEnv();
            for (auto& stmt : elif.body) stmt->accept(this);
            popEnv();
            return;
        }
    }
    
    if (!node->elseBranch.empty()) {
        pushEnv();
        for (auto& stmt : node->elseBranch) stmt->accept(this);
        popEnv();
    }
}

void Evaluator::visit(ForStatement* node) {
    pushEnv();
    
    // Init
    RuntimeValue initVal = evaluateExpr(node->initValue.get());
    env->assignVariable(node->initTarget, initVal);
    
    while (true) {
        RuntimeValue condVal = evaluateExpr(node->condition.get());
        if (condVal.type != DataType::BOOL) throw std::runtime_error("FOR condition must be BOOL.");
        if (!std::get<bool>(condVal.value)) break;
        
        pushEnv();
        for (auto& stmt : node->body) stmt->accept(this);
        popEnv();
        
        RuntimeValue upVal = evaluateExpr(node->updateValue.get());
        env->assignVariable(node->updateTarget, upVal);
    }
    
    popEnv();
}

void Evaluator::visit(RepeatStatement* node) {
    while (true) {
        RuntimeValue condVal = evaluateExpr(node->condition.get());
        if (condVal.type != DataType::BOOL) throw std::runtime_error("REPEAT condition must be BOOL.");
        if (!std::get<bool>(condVal.value)) break;
        
        pushEnv();
        for (auto& stmt : node->body) stmt->accept(this);
        popEnv();
    }
}

void Evaluator::visit(BinaryExpr* node) {
    RuntimeValue left = evaluateExpr(node->left.get());
    RuntimeValue right = evaluateExpr(node->right.get());
    
    auto t = node->op.type;
    
    // Logical
    if (t == TokenType::AND || t == TokenType::OR) {
        if (left.type != DataType::BOOL || right.type != DataType::BOOL) throw std::runtime_error("Logical ops require BOOL.");
        bool l = std::get<bool>(left.value);
        bool r = std::get<bool>(right.value);
        if (t == TokenType::AND) currentResult = RuntimeValue(l && r);
        else currentResult = RuntimeValue(l || r);
        return;
    }
    
    // Equality
    if (t == TokenType::EQUAL_EQUAL || t == TokenType::NOT_EQUAL) {
        bool eq = false;
        if (left.type == DataType::INT && right.type == DataType::INT) eq = (std::get<int>(left.value) == std::get<int>(right.value));
        else if (left.type == DataType::FLOAT && right.type == DataType::FLOAT) eq = (std::get<float>(left.value) == std::get<float>(right.value));
        else if (left.type == DataType::CHAR && right.type == DataType::CHAR) eq = (std::get<char>(left.value) == std::get<char>(right.value));
        else if (left.type == DataType::BOOL && right.type == DataType::BOOL) eq = (std::get<bool>(left.value) == std::get<bool>(right.value));
        else {
            // Mixed INT/FLOAT fallback
            if (left.type == DataType::INT && right.type == DataType::FLOAT) eq = (std::get<int>(left.value) == std::get<float>(right.value));
            else if (left.type == DataType::FLOAT && right.type == DataType::INT) eq = (std::get<float>(left.value) == std::get<int>(right.value));
        }
        
        if (t == TokenType::NOT_EQUAL) eq = !eq;
        currentResult = RuntimeValue(eq);
        return;
    }
    
    // Comparison
    if (t == TokenType::GREATER || t == TokenType::LESS || t == TokenType::GREATER_EQUAL || t == TokenType::LESS_EQUAL) {
        float l = 0, r = 0;
        if (left.type == DataType::INT) l = std::get<int>(left.value); else l = std::get<float>(left.value);
        if (right.type == DataType::INT) r = std::get<int>(right.value); else r = std::get<float>(right.value);
        
        bool res = false;
        if (t == TokenType::GREATER) res = l > r;
        if (t == TokenType::LESS) res = l < r;
        if (t == TokenType::GREATER_EQUAL) res = l >= r;
        if (t == TokenType::LESS_EQUAL) res = l <= r;
        currentResult = RuntimeValue(res);
        return;
    }
    
    // Arithmetic
    if (t == TokenType::PLUS || t == TokenType::MINUS || t == TokenType::STAR || t == TokenType::SLASH || t == TokenType::MODULO) {
        if (left.type == DataType::FLOAT || right.type == DataType::FLOAT) {
            float l = (left.type == DataType::INT) ? std::get<int>(left.value) : std::get<float>(left.value);
            float r = (right.type == DataType::INT) ? std::get<int>(right.value) : std::get<float>(right.value);
            float res = 0;
            switch(t) {
                case TokenType::PLUS: res = l + r; break;
                case TokenType::MINUS: res = l - r; break;
                case TokenType::STAR: res = l * r; break;
                case TokenType::SLASH: res = l / r; break;
                case TokenType::MODULO: res = std::fmod(l, r); break;
                default: break;
            }
            currentResult = RuntimeValue(res);
            return;
        } else {
            int l = std::get<int>(left.value);
            int r = std::get<int>(right.value);
            int res = 0;
            switch(t) {
                case TokenType::PLUS: res = l + r; break;
                case TokenType::MINUS: res = l - r; break;
                case TokenType::STAR: res = l * r; break;
                case TokenType::SLASH: res = l / r; break;
                case TokenType::MODULO: res = l % r; break;
                default: break;
            }
            currentResult = RuntimeValue(res);
            return;
        }
    }
}

void Evaluator::visit(UnaryExpr* node) {
    RuntimeValue right = evaluateExpr(node->right.get());
    if (node->op.type == TokenType::NOT) {
        if (right.type != DataType::BOOL) throw std::runtime_error("NOT requires BOOL.");
        currentResult = RuntimeValue(!std::get<bool>(right.value));
    } else if (node->op.type == TokenType::MINUS) {
        if (right.type == DataType::INT) currentResult = RuntimeValue(-std::get<int>(right.value));
        else if (right.type == DataType::FLOAT) currentResult = RuntimeValue(-std::get<float>(right.value));
        else throw std::runtime_error("MINUS requires number.");
    } else if (node->op.type == TokenType::PLUS) {
        currentResult = right;
    }
}

void Evaluator::visit(LiteralExpr* node) {
    if (node->value.type == TokenType::INT_LITERAL) {
        currentResult = RuntimeValue(std::stoi(node->value.value));
    } else if (node->value.type == TokenType::FLOAT_LITERAL) {
        currentResult = RuntimeValue(std::stof(node->value.value));
    } else if (node->value.type == TokenType::BOOL_LITERAL) {
        currentResult = RuntimeValue(node->value.value == "TRUE");
    } else if (node->value.type == TokenType::CHAR_LITERAL) {
        currentResult = RuntimeValue(node->value.value[0]);
    } else if (node->value.type == TokenType::STRING_LITERAL) {
        currentResult = RuntimeValue(node->value.value);
    } else if (node->value.type == TokenType::DOLLAR) { // Treat dollar literal as "$", handled by Print
        currentResult = RuntimeValue(std::string("$")); 
    }
}

void Evaluator::visit(IdentifierExpr* node) {
    currentResult = env->getVariable(node->name);
}
