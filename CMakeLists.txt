cmake_minimum_required(VERSION 3.20)
project(KN0-LANG CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(KN0-LANG
    src/main.cpp
"includes/Tokens.h"
"includes/Lexer.h"
"src/Lexer.cpp"
"includes/Expr.h"
"includes/Parser.h"
"src/Parser.cpp"
"includes/stmt.h"
"includes/Instruction.h"
"includes/Compiler.h"
"src/Compiler.cpp"
"includes/Machine.h"
"src/Machine.cpp"
)

target_include_directories(KN0-LANG PRIVATE includes)