#include "Lexer.h"

#include <format>
#include <iostream>
#include <stdexcept>
using namespace Lexing;

const std::unordered_map<std::string_view, TokenType> Lexer::known_words = {
    {"const"sv, TokenType::Const}, {"var"sv, TokenType::Var}, {"let"sv, TokenType::Let},
    {"if"sv, TokenType::If}, {"else"sv, TokenType::Else}, {"switch"sv, TokenType::Switch},
    {"case"sv, TokenType::Case}, {"default"sv, TokenType::Default}, {"return"sv, TokenType::Return},
    {"break"sv, TokenType::Break}, {"continue"sv, TokenType::Continue}, {"class"sv, TokenType::Class},
    {"true"sv, TokenType::True}, {"false"sv, TokenType::False}, {"while"sv, TokenType::While},
    {"for"sv, TokenType::For}, {"function"sv, TokenType::Function}
};

static consteval std::array<TokenType, 128> Lexing::generate_single_char_known_words() {
    std::array<TokenType, 128> array = {};

    for (size_t i = 0; i < 128; ++i) {
        array[i] = TokenType::None;
    }

    array['{'] = TokenType::OpenBrace;
    array['}'] = TokenType::CloseBrace;
    array['('] = TokenType::OpenParen;
    array[')'] = TokenType::CloseParen;
    array['['] = TokenType::OpenBracket;
    array[']'] = TokenType::CloseBracket;
    array[','] = TokenType::Comma;
    array[';'] = TokenType::SemiColon;
    array['\"'] = TokenType::DoubleQuote;
    array['\''] = TokenType::SingleQuote;
    array['.'] = TokenType::Dot;
    array['='] = TokenType::Equals;
    array['#'] = TokenType::HashTag;

    array['+'] = array['-'] = array['*']
        = array['/'] = array['>'] = array['<'] = array['&'] = array['|'] = array['^'] = TokenType::BinaryOperator;
    array['!'] = TokenType::Operator;

    return array;
}

static consteval std::array<bool, 128> Lexing::generate_skip_chars() {
    std::array<bool, 128> array = {};

    for (size_t i = 0; i < 128; i++) {
        array[i] = false;
    }

    array[' '] = true;
    array['\t'] = true;
    array['\n'] = true;
    array['\r'] = true;

    return array;
};

const std::array<TokenType, 128> Lexer::single_char_known_symbols = generate_single_char_known_words();
const std::unordered_map<std::string_view, TokenType> Lexer::extended_char_known_symbols = {
    {"<="sv, TokenType::BinaryOperator}, {">="sv, TokenType::BinaryOperator},
    {"=="sv, TokenType::BinaryOperator},{"!="sv, TokenType::BinaryOperator},
    {"!="sv, TokenType::BinaryOperator},{"!=="sv, TokenType::BinaryOperator},
    {"==="sv, TokenType::BinaryOperator}, {"<<"sv, TokenType::BinaryOperator},
    {">>"sv, TokenType::BinaryOperator}, {"||"sv, TokenType::BinaryOperator},
    {"&&"sv, TokenType::BinaryOperator}
};

const std::array<bool, 128> Lexer::skip_chars = generate_skip_chars();

std::vector<Token> Lexer::tokenize(const char* code, size_t len) {
    if (code[0] == '\0')
        return {};

    current_code = code;
    current_index = 0;
    current_char = code[0];

    std::vector<Token> tokens = {};
    while (current_char != '\0') {
        while (skip_chars[current_char]) advance();
        if (current_char == '\0') break;

        if (isalpha(current_char) || current_char == '_') {
            size_t len = 0;
            const size_t index = current_index;

            while (isalpha(current_char) || isdigit(current_char) || current_char == '_') {
                len++;
                advance();
            }

            const std::string_view str_view = {&current_code[index], len};

            if (const auto itr = known_words.find(str_view); itr != known_words.end())
                tokens.push_back(Token{itr->second, str_view});
            else tokens.push_back(Token{TokenType::Identifier, str_view});

        } else if (isdigit(current_char)) {
            size_t len = 0;
            const size_t index = current_index;

            while (isdigit(current_char) || current_char == '.' || std::tolower(current_char) == 'f') {
                len++;
                advance();
            }

            const std::string_view str_view = {&current_code[index], len};
            tokens.push_back(Token{TokenType::Number, str_view});

        } else {
            if (current_index + 2 < len) {
                std::string_view str_view = {&current_code[current_index], 3};

                if (const auto itr = extended_char_known_symbols.find(str_view); itr != extended_char_known_symbols.end()) {
                    tokens.push_back(Token{itr->second, str_view});
                    advance();
                    advance();
                    advance();
                    continue;
                }
            }
            if (current_index + 1 < len) {
                std::string_view str_view = {&current_code[current_index], 2};

                if (const auto itr = extended_char_known_symbols.find(str_view); itr != extended_char_known_symbols.end()) {
                    tokens.push_back(Token{itr->second, str_view});
                    advance();
                    advance();
                    continue;
                }
            }
            if (single_char_known_symbols[current_char] != TokenType::None) {
                tokens.push_back(Token{single_char_known_symbols[current_char],
                    std::string_view{&current_code[current_index], 1}});
                advance();
                continue;
            }

            throw std::runtime_error(std::format("UnExpected Token {}", code[current_index]));
        }
    }

    current_code = "\0";
    current_index = 0;
    current_char = '\0';

    return tokens;
}

void Lexer::advance() {
    if (current_char != '\0') {
        current_char = current_code[++current_index];
    }
}
