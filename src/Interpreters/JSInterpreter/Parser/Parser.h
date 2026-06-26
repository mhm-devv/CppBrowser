#pragma once
#include "Ast.h"
#include "../Lexer/Lexer.h"

namespace Parsing {
    class Parser {
    public:
        std::unique_ptr<Ast::Program> produceAst(const std::vector<Lexing::Token>& tokens);

        std::unique_ptr<Ast::Statement> parseStatement();
        std::unique_ptr<Ast::VarDecl> parseVarDecl();
        std::unique_ptr<Ast::VarReInit> parseVarReInit(std::unique_ptr<Ast::Identifier> identifier);
        std::unique_ptr<Ast::If> parseIf();
        std::unique_ptr<Ast::While> parseWhile();
        std::unique_ptr<Ast::For> parseFor();
        std::unique_ptr<Ast::FuncDecl> parseFuncDecl();
        std::unique_ptr<Ast::IndexReInit> parseIndexReInit(std::unique_ptr<Ast::IndexAccess> expr);
        std::unique_ptr<Ast::MemberReInit> parseMemberReInit(std::unique_ptr<Ast::MemberAccess> expr);
        std::unique_ptr<Ast::ScopeDecl> parseScopeDecl();

        std::unique_ptr<Ast::Expression> parseExpression(int min_precedence = 0);
        std::unique_ptr<Ast::Expression> parseNud();
        std::unique_ptr<Ast::Expression> parseParens();
        std::unique_ptr<Ast::Number> parseNumber();
        std::unique_ptr<Ast::String> parseString();
        std::unique_ptr<Ast::Boolean> parseBoolean();
        std::unique_ptr<Ast::Array> parseArray();
        std::unique_ptr<Ast::Object> parseObject();
        std::unique_ptr<Ast::FuncCall> parseFuncCall(std::unique_ptr<Ast::Expression> expr);
        std::unique_ptr<Ast::IndexAccess> parseIndexAccess(std::unique_ptr<Ast::Expression> expr);
        std::unique_ptr<Ast::MemberAccess> parseMemberAccess(std::unique_ptr<Ast::Expression> expr);

    private:
        static const std::unordered_map<std::string_view, int> precedences;
        std::vector<Lexing::Token>::const_iterator current_token;
        const Lexing::Token& advance();
    };
}
