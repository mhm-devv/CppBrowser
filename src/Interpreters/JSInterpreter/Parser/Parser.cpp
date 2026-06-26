#include "Parser.h"

#include <functional>

using namespace Parsing;
using namespace Lexing;

const std::unordered_map<std::string_view, int> Parser::precedences = {
    {"<"sv, 5}, {">"sv, 5}, {"<="sv, 5}, {">="sv, 5}, {"=="sv, 5}, {"==="sv, 5}, {"!="sv, 5}, {"!=="sv, 5},
    {"+"sv, 10}, {"-"sv, 10}, {"*"sv, 15}, {"/"sv, 15}, {"%"sv, 15}
};

std::unique_ptr<Ast::Program> Parser::produceAst(const std::vector<Lexing::Token>& tokens) {
    std::vector<std::unique_ptr<Ast::Statement>> statements;

    current_token = tokens.begin();
    while (current_token->type != TokenType::EndOfFile) {
        statements.push_back(parseStatement());
    }

    return std::make_unique<Ast::Program>(std::move(statements));
}

std::unique_ptr<Ast::Statement> Parser::parseStatement() {
    auto parse_statement_lambda = [this] -> std::unique_ptr<Ast::Statement> {
        while (current_token->type == TokenType::SemiColon) advance();
        switch (current_token->type) {
            case TokenType::Const: case TokenType::Let: case TokenType::Var: return parseVarDecl();
            case TokenType::If: return parseIf();
            case TokenType::While: return parseWhile();
            case TokenType::For: return parseFor();
            case TokenType::Function: return parseFuncDecl();
            case TokenType::OpenBrace: return parseScopeDecl();
            default: return parseExpression();
        }
    };

    std::unique_ptr<Ast::Statement> statement = parse_statement_lambda();
    if (statement->type == Ast::StatementType::Expr && current_token->type == TokenType::Equals) {
        auto* expr = dynamic_cast<Ast::Expression*>(statement.release());
        if (expr->type == Ast::ExpressionType::Identifier) {
            statement = parseVarReInit(std::unique_ptr<Ast::Identifier>{dynamic_cast<Ast::Identifier*>(expr)});
        } else if (expr->type == Ast::ExpressionType::IndexAccess) {
            statement = parseIndexReInit(std::unique_ptr<Ast::IndexAccess>{dynamic_cast<Ast::IndexAccess*>(expr)});
        } else if (expr->type == Ast::ExpressionType::MemberAccess) {
            statement = parseMemberAccess(std::unique_ptr<Ast::MemberAccess>{dynamic_cast<Ast::MemberAccess*>(expr)});
        } else throw std::runtime_error("unexpected =");
    }

    if (current_token->type == TokenType::SemiColon) advance();

    return statement;
}
std::unique_ptr<Ast::ScopeDecl> Parser::parseScopeDecl() {
    advance(); // {
    std::vector<std::unique_ptr<Ast::Statement>> statements;
    while (current_token->type != TokenType::CloseBrace) {
        statements.push_back(parseStatement());
    }
    advance(); // }

    return std::make_unique<Ast::ScopeDecl>(std::move(statements));
}
std::unique_ptr<Ast::VarDecl> Parser::parseVarDecl() {
    const bool is_const = advance().type == TokenType::Const;
    const std::string_view var_name = advance().str;

    if (current_token->type == TokenType::Equals) {
        advance(); // =
        std::unique_ptr<Ast::Expression> value = parseExpression();

        return std::make_unique<Ast::VarDecl>(is_const, var_name, std::move(value));
    }

    return std::make_unique<Ast::VarDecl>(is_const, var_name, nullptr);
}
std::unique_ptr<Ast::VarReInit> Parser::parseVarReInit(std::unique_ptr<Ast::Identifier> identifier) {
    advance(); // =
    std::unique_ptr<Ast::Expression> value = parseExpression();

    return std::make_unique<Ast::VarReInit>(identifier->identifier, std::move(value));
}
std::unique_ptr<Ast::If> Parser::parseIf() {
    advance(); // if
    std::unique_ptr<Ast::Expression> if_condition = parseExpression();
    std::unique_ptr<Ast::Statement> if_statement = parseStatement();

    std::vector<std::unique_ptr<Ast::ElseIf>> else_ifs;
    while (current_token->type == TokenType::Else && (current_token + 1)->type == TokenType::If) {
        advance(); // else
        advance(); // if
        std::unique_ptr<Ast::Expression> else_if_condition = parseExpression();
        std::unique_ptr<Ast::Statement> else_if_statement = parseStatement();
        else_ifs.push_back(std::make_unique<Ast::ElseIf>(std::move(else_if_condition), std::move(else_if_statement)));
    }

    std::unique_ptr<Ast::Statement> else_statement = nullptr;

    if (current_token->type == TokenType::Else) {
        advance(); // else
        else_statement = parseStatement();
    }

    return std::make_unique<Ast::If>(std::move(if_condition), std::move(if_statement), std::move(else_ifs), std::move(else_statement));
}
std::unique_ptr<Ast::While> Parser::parseWhile() {
    advance(); // while
    std::unique_ptr<Ast::Expression> while_condition = parseExpression();
    std::unique_ptr<Ast::Statement> while_statement = parseStatement();

    return std::make_unique<Ast::While>(std::move(while_condition), std::move(while_statement));
}

// fix later
std::unique_ptr<Ast::For> Parser::parseFor() {
    advance(); // for
    advance(); // (

    std::unique_ptr<Ast::VarDecl> first_statement = current_token->type != TokenType::SemiColon ? parseVarDecl() : nullptr;
    std::unique_ptr<Ast::Expression> condition = current_token->type != TokenType::SemiColon ? parseExpression() : nullptr;
    std::unique_ptr<Ast::Statement> after_statement = current_token->type != TokenType::SemiColon ? parseStatement() : nullptr;

    advance(); // )

    std::unique_ptr<Ast::Statement> statement = parseStatement();

    return std::make_unique<Ast::For>(std::move(first_statement), std::move(condition),
        std::move(after_statement), std::move(statement));
}
std::unique_ptr<Ast::FuncDecl> Parser::parseFuncDecl() {
    advance(); // function
    const std::string_view func_name = advance().str;

    advance(); // (
    std::vector<std::string_view> args;

    while (current_token->type != TokenType::CloseParen) {
        args.push_back(advance().str);
        if (current_token->type == TokenType::Comma)
            advance();
        else break;
    }

    advance(); // )

    std::unique_ptr<Ast::Statement> statement = parseExpression();

    return std::make_unique<Ast::FuncDecl>(func_name, std::move(args), std::move(statement));
}
std::unique_ptr<Ast::IndexReInit> Parser::parseIndexReInit(std::unique_ptr<Ast::IndexAccess> expr) {
    advance();
    std::unique_ptr<Ast::Expression> value = parseExpression();

    return std::make_unique<Ast::IndexReInit>(std::move(expr), std::move(value));
}
std::unique_ptr<Ast::MemberReInit> Parser::parseMemberReInit(std::unique_ptr<Ast::MemberAccess> expr) {
    advance();
    std::unique_ptr<Ast::Expression> value = parseExpression();

    return std::make_unique<Ast::MemberReInit>(std::move(expr), std::move(value));
};

std::unique_ptr<Ast::Expression> Parser::parseExpression(int min_precedence) {
    std::unique_ptr<Ast::Expression> left = parseNud();

    while (true) {
        if (const auto itr = precedences.find(current_token->str); itr != precedences.end()) {
            if (itr->second < min_precedence) break;

            const std::string_view op = advance().str;
            std::unique_ptr<Ast::Expression> right = parseExpression(itr->second);
            left = std::make_unique<Ast::BinaryExpr>(std::move(left), op, std::move(right));
        } else break;
    }

    return left;
}
std::unique_ptr<Ast::Expression> Parser::parseNud() {
    auto get_expr_lambda = [this] -> std::unique_ptr<Ast::Expression> {
        switch (current_token->type) {
            case TokenType::Number: return parseNumber();
            case TokenType::DoubleQuote: return parseString();
            case TokenType::True: case TokenType::False: return parseBoolean();
            case TokenType::OpenBracket: return parseArray();
            case TokenType::OpenBrace: return parseObject();
            case TokenType::Identifier: return std::make_unique<Ast::Identifier>(advance().str);
            case TokenType::OpenParen: return parseParens();
            default: throw std::runtime_error("UnExpected Token In ParseNud");
        }
    };

    std::unique_ptr<Ast::Expression> expr = get_expr_lambda();

    while (true) {
        if (current_token->type == TokenType::OpenParen) {
            expr = parseFuncCall(std::move(expr));
            continue;
        }
        if (current_token->type == TokenType::OpenBracket) {
            expr = parseIndexAccess(std::move(expr));
            continue;
        }
        if (current_token->type == TokenType::Dot) {
            expr = parseMemberAccess(std::move(expr));
            continue;
        }

        break;
    }

    return expr;
}
std::unique_ptr<Ast::Number> Parser::parseNumber() {
    float val;
    auto [ptr, ec] = std::from_chars(current_token->str.data(), current_token->str.data() + current_token->str.size(), val);
    advance();
    return std::make_unique<Ast::Number>(val);
}
std::unique_ptr<Ast::String> Parser::parseString() {
    advance();
    const std::string_view str = advance().str;
    advance();

    return std::make_unique<Ast::String>(str);
}
std::unique_ptr<Ast::Boolean> Parser::parseBoolean() {
    return std::make_unique<Ast::Boolean>(advance().type == TokenType::True);
}
std::unique_ptr<Ast::Array> Parser::parseArray() {
    advance();
    std::vector<std::unique_ptr<Ast::Expression>> exprs;

    while (current_token->type != TokenType::CloseBracket) {
        exprs.push_back(parseExpression());
        if (current_token->type == TokenType::Comma)
            advance();
        else break;
    }

    advance();

    return std::make_unique<Ast::Array>(std::move(exprs));
}
std::unique_ptr<Ast::Expression> Parser::parseParens() {
    advance(); // (
    std::unique_ptr<Ast::Expression> expr = parseExpression();
    advance(); // )

    return expr;
}
std::unique_ptr<Ast::Object> Parser::parseObject() {
    advance(); // {
    std::unordered_map<std::string_view, std::unique_ptr<Ast::Expression>> values;

    while (current_token->type != TokenType::CloseBrace) {
        const std::string_view key = advance().str;
        advance(); // :
        std::unique_ptr<Ast::Expression> value = parseExpression();
        values.insert({key, std::move(value)});
        if (current_token->type == TokenType::Comma)
            advance();
        else break;
    }
    advance(); // }

    return std::make_unique<Ast::Object>(std::move(values));
}
std::unique_ptr<Ast::FuncCall> Parser::parseFuncCall(std::unique_ptr<Ast::Expression> expr) {
    advance(); // (
    std::vector<std::unique_ptr<Ast::Expression>> args;
    while (current_token->type != TokenType::CloseParen) {
        args.push_back(parseExpression());
        if (current_token->type == TokenType::Comma)
            advance();
        else break;
    }
    advance(); // )

    return std::make_unique<Ast::FuncCall>(std::move(expr), std::move(args));
}
std::unique_ptr<Ast::IndexAccess> Parser::parseIndexAccess(std::unique_ptr<Ast::Expression> expr) {
    advance(); // [
    std::unique_ptr<Ast::Expression> index_expr = parseExpression();
    advance(); // ]

    return std::make_unique<Ast::IndexAccess>(std::move(expr), std::move(index_expr));
}
std::unique_ptr<Ast::MemberAccess> Parser::parseMemberAccess(std::unique_ptr<Ast::Expression> expr) {
    advance(); // .

    return std::make_unique<Ast::MemberAccess>(std::move(expr), advance().str);
}

const Lexing::Token& Parser::advance() {
    if (current_token->type != TokenType::EndOfFile) {
        const Lexing::Token& old_token = *current_token;
        ++current_token;
        return old_token;
    }
    return *current_token;
};