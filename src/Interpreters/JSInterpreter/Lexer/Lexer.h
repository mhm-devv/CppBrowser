#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace Lexing {
    using namespace std::string_view_literals;

    enum class TokenType {
        None,
        // Symbols
        OpenBrace, CloseBrace, OpenBracket, CloseBracket, OpenParen, CloseParen,
        Comma, SemiColon, Colon, DoubleQuote, SingleQuote, Dot, Operator, BinaryOperator,
        Equals, HashTag,

        // Words
        Const, Var, Let, If, Else, Switch, Case, Default, Return, Continue, Break,
        Class, Identifier, Number, True, False, While, For, Function,

        EndOfFile
    };

    struct Token {
        TokenType type;
        std::string_view str;
    };

    static consteval std::array<TokenType, 128> generate_single_char_known_words();
    static consteval std::array<bool, 128> generate_skip_chars();

    class Lexer {
    public:
        static const std::unordered_map<std::string_view, TokenType> known_words;
        static const std::array<TokenType, 128> single_char_known_symbols;
        static const std::unordered_map<std::string_view, TokenType> extended_char_known_symbols;
        static const std::array<bool, 128> skip_chars;

        std::vector<Token> tokenize(const char* code, size_t len);
    private:
        size_t current_index = 0;
        char current_char = '\0';
        const char* current_code = "\0";

        void advance();
    };
}
