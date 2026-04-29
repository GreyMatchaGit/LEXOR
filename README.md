# LEXOR

Programming Language class submission requirement

**Course**: CS322 - Programming Languages

### Developers:
- Karl Phoenix Cornilla (GreyMatchaGit)
- Sydney Galorio (Shizune-23)

## Architecture Summary

The LEXOR Interpreter evaluates programs via a classic sequential interpretation pipeline crafted uniformly in C++17. 

1. **Lexer**: Iterates through the raw source file to produce valid `Token` streams. The Lexer uses context-awareness behavior, validating and extracting leading tabs/spaces into `INDENT` and `DEDENT` tokens specifically to enforce proper visual blocks, alongside detecting strings, special escape mappings (`[]`), and keywords.
2. **Abstract Syntax Tree (AST)**: Utilizes the **Composite Design Pattern**. The syntax tree establishes base `ASTNode` derivations split into lightweight `Statement` clusters and nested `Expression` components.
3. **Parser**: Translates generated token lines using a recursive descent algorithm structure. The parser inherently controls boundaries, throwing strict parsing context violations and verifying mandatory hierarchy like `DECLARE` bounds strictly falling beneath an indented `START SCRIPT`. 
4. **Evaluator**: Interprets and executes the compiled `AST` utilizing the **Visitor Design Pattern**. Nodes cleanly accept Evaluation contexts without inline operations. All runtime state is allocated against an `Environment` scope object capable of nesting environments block-by-block while safely validating dynamic C++ `std::variant` wrappers to verify memory and operand types (such as rigorous `BOOL` handling and Math combinations).

## Compilation Instructions

To compile the LEXOR interpreter, use a C++17 compatible compiler. Run the following command from the root directory of the project:

```bash
g++ -std=c++17 src/main.cpp src/structs/*.cpp src/components/*.cpp -o lexor
```

This will generate the `lexor` executable. You can then run a script or launch the interactive REPL:

```bash
./lexor test/sample3.lx
./lexor
```
