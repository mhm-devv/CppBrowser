#include <cstring>

#include "src/Interpreters/JSInterpreter/Lexer/Lexer.h"

int main() {
    Lexing::Lexer lexer;
    const char* repeated_str = "const HELLO = THERE;\n";
    lexer.tokenize(repeated_str, std::strlen(repeated_str));
}
