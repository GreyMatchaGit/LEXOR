#include "environment.h"
#include <stdexcept>

Environment::Environment(Environment* parentEnv) : parent(parentEnv) {}

Environment::~Environment() {}

void Environment::declareVariable(const std::string& name, RuntimeValue value) {
    if (variables.find(name) != variables.end()) {
        throw std::runtime_error("Variable already declared: " + name);
    }
    variables[name] = value;
}

void Environment::assignVariable(const std::string& name, RuntimeValue value) {
    if (variables.find(name) != variables.end()) {
        if (variables[name].type != value.type) {
            throw std::runtime_error("Type mismatch assignment for variable: " + name);
        }
        variables[name] = value;
        return;
    }
    if (parent != nullptr) {
        parent->assignVariable(name, value);
        return;
    }
    throw std::runtime_error("Variable not found for assignment: " + name);
}

RuntimeValue Environment::getVariable(const std::string& name) {
    if (variables.find(name) != variables.end()) {
        return variables[name];
    }
    if (parent != nullptr) {
        return parent->getVariable(name);
    }
    throw std::runtime_error("Variable not found: " + name);
}

bool Environment::checkExists(const std::string& name) {
    if (variables.find(name) != variables.end()) {
        return true;
    }
    if (parent != nullptr) {
        return parent->checkExists(name);
    }
    return false;
}
