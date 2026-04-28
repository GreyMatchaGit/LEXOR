#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "runtime_value.h"
#include <unordered_map>
#include <string>

class Environment {
private:
    std::unordered_map<std::string, RuntimeValue> variables;
    Environment* parent;

public:
    Environment(Environment* parentEnv = nullptr);
    ~Environment();

    void declareVariable(const std::string& name, RuntimeValue value);
    void assignVariable(const std::string& name, RuntimeValue value);
    RuntimeValue getVariable(const std::string& name);
    bool checkExists(const std::string& name);
    
    Environment* getParent() { return parent; }
};

#endif
