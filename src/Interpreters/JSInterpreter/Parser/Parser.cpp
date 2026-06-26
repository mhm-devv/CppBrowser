#include "Parser.h"

using namespace Parsing;

std::unique_ptr<Ast::Program> Parser::produceAst(const std::vector<Lexing::Token>& tokens) {
    std::vector<std::unique_ptr<Ast::Statement>> statements;

    current_token = tokens.begin();
    while (current_token != tokens.end()) {
        statements.push_back(parseStatement());
    }

    return std::make_unique<Ast::Program>(std::move(statements));
}

std::unique_ptr<Ast::Statement> Parser::parseStatement() {
    switch (current_token->type) {

    }
}
std::unique_ptr<Ast::VarDecl> Parser::parseVarDecl() {}
std::unique_ptr<Ast::VarReInit> Parser::parseVarReInit() {}
std::unique_ptr<Ast::If> Parser::parseIf() {}
std::unique_ptr<Ast::While> Parser::parseWhile() {}
std::unique_ptr<Ast::For> Parser::parseFor() {}
std::unique_ptr<Ast::FuncDecl> Parser::parseFuncDecl() {}

std::unique_ptr<Ast::Expression> Parser::parseExpression() {}
std::unique_ptr<Ast::Number> Parser::parseNumber() {}
std::unique_ptr<Ast::String> Parser::parseString() {}
std::unique_ptr<Ast::Boolean> Parser::parseBoolean() {}
std::unique_ptr<Ast::Array> Parser::parseArray() {}
std::unique_ptr<Ast::Object> Parser::parseObject() {}
std::unique_ptr<Ast::FuncCall> Parser::parseFuncCall() {}
std::unique_ptr<Ast::IndexAccess> Parser::parseIndexAccess() {}
std::unique_ptr<Ast::MemberAccess> Parser::parseMemberAccess() {}