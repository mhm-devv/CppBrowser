cmake_minimum_required(VERSION 4.2)
project(Browser)

set(CMAKE_CXX_STANDARD 26)

add_executable(Browser main.cpp
        src/Interpreters/JSInterpreter/Lexer/Lexer.h
        src/Interpreters/JSInterpreter/Lexer/Lexer.cpp
        src/Interpreters/JSInterpreter/Parser/Ast.h
        src/Interpreters/JSInterpreter/Parser/Parser.h
        src/Interpreters/JSInterpreter/Parser/Parser.cpp
        src/Interpreters/JSInterpreter/IL/IL.h
        src/Interpreters/JSInterpreter/IL/IL.cpp)
