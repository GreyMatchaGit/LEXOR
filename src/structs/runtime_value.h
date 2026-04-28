#ifndef RUNTIME_VALUE_H
#define RUNTIME_VALUE_H

#include <string>
#include <variant>

enum class DataType {
    INT,
    FLOAT,
    CHAR,
    BOOL,
    STRING // For strings/intermediate usage
};

struct RuntimeValue {
    DataType type;
    std::variant<int, float, char, bool, std::string> value;
    
    RuntimeValue() : type(DataType::INT), value(0) {}
    RuntimeValue(int v) : type(DataType::INT), value(v) {}
    RuntimeValue(float v) : type(DataType::FLOAT), value(v) {}
    RuntimeValue(char v) : type(DataType::CHAR), value(v) {}
    RuntimeValue(bool v) : type(DataType::BOOL), value(v) {}
    RuntimeValue(const std::string& v) : type(DataType::STRING), value(v) {}
};

#endif
