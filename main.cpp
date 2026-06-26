#include <cstring>

#include "src/Interpreters/JSInterpreter/Lexer/Lexer.h"
#include "src/Interpreters/JSInterpreter/Parser/Parser.h"

int main() {
    Lexing::Lexer lexer;
    const char* repeated_str = "const mhm = 0\nlet r = 10\n if(mhm == 10) {print(\"hello\")}";
    std::vector<Lexing::Token> tokens = lexer.tokenize(repeated_str, std::strlen(repeated_str));
    Parsing::Parser parser;
    parser.produceAst(tokens);
}
