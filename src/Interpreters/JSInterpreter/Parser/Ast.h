#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

namespace Ast {
    enum class StatementType {
        VarDecl, VarReInit, If, ElseIf, Switch, Case, Default,
        Break, Continue, Return, While, For, FuncDecl, ClassDecl, Expr, Program, Scope,
        IndexReInit, MemberReInit
    };
    enum class ExpressionType {
        Number, String, Boolean, Func, Object, Array, FuncCall, IndexAccess, MemberAccess
    };

    class Statement {
    public:
        const StatementType type;
        Statement(const StatementType stmt_type): type(stmt_type) {}
    };
    class Expression : public Statement {
    public:
        const ExpressionType type;
        Expression(const ExpressionType expr_type): Statement(StatementType::Expr), type(expr_type) {}
    };

    class Program : public Statement {
    public:
        const std::vector<std::unique_ptr<Statement>> statements;
        Program(std::vector<std::unique_ptr<Statement>> stmts): Statement(StatementType::Program), statements(std::move(stmts)) {}
    };
    class ScopeDecl : public Statement {
    public:
        const std::vector<std::unique_ptr<Statement>> statements;
        ScopeDecl(std::vector<std::unique_ptr<Statement>> stmts): Statement(StatementType::Scope), statements(std::move(stmts)) {}
    };
    class VarDecl : public Statement {
    public:
        const bool is_const;
        const std::string_view name;
        const std::unique_ptr<Expression> value;
        VarDecl(bool is_const_, std::string_view name_, std::unique_ptr<Expression> val):
            Statement(StatementType::VarDecl), name(name_), value(std::move(val)), is_const(is_const_) {}
    };
    class VarReInit : public Statement {
    public:
        const std::string_view name;
        const std::unique_ptr<Expression> value;
        VarReInit(std::string_view name_, std::unique_ptr<Expression> val):
            Statement(StatementType::VarReInit), name(name_), value(std::move(val)) {}
    };
    class ElseIf : public Statement {
    public:
        const std::unique_ptr<Expression> condition;
        const std::unique_ptr<Statement> statement;
        ElseIf(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> stmt): Statement(StatementType::ElseIf),
            condition(std::move(cond)), statement(std::move(stmt)) {}
    };
    class If : public Statement {
    public:
        const std::unique_ptr<Expression> condition;
        const std::unique_ptr<Statement> statement;
        const std::vector<std::unique_ptr<ElseIf>> else_if_stmts;
        const std::unique_ptr<Statement> else_stmt;
        If(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> stmt, std::vector<std::unique_ptr<ElseIf>> else_ifs,
            std::unique_ptr<Statement> else_): Statement(StatementType::If), condition(std::move(cond)),
            statement(std::move(stmt)), else_if_stmts(std::move(else_ifs)), else_stmt(std::move(else_)) {}
    };
    class While : public Statement {
    public:
        const std::unique_ptr<Expression> condition;
        const std::unique_ptr<Statement> statement;
        While(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> stmt): Statement(StatementType::While),
            condition(std::move(cond)), statement(std::move(stmt)) {}
    };
    class For : public Statement {
    public:
        const std::unique_ptr<VarDecl> var_decleration;
        const std::unique_ptr<Expression> condition;
        const std::unique_ptr<Statement> after_statement;
        const std::unique_ptr<Statement> statement;
        For(std::unique_ptr<VarDecl> var_decl, std::unique_ptr<Expression> cond,
            std::unique_ptr<Statement> after_stmt, std::unique_ptr<Statement> stmt):
            Statement(StatementType::For), var_decleration(std::move(var_decl)), condition(std::move(cond)),
            after_statement(std::move(after_stmt)), statement(std::move(stmt)) {}
    };
    class FuncDecl : public Statement {
    public:
        const std::string_view name;
        const std::vector<std::string_view> args;
        const std::unique_ptr<Statement> statement;
        FuncDecl(std::string_view name_, std::vector<std::string_view> args_, std::unique_ptr<Statement> stmt):
            Statement(StatementType::FuncDecl), name(name_), args(std::move(args_)), statement(std::move(stmt)) {}
    };
    class Break : public Statement {
    public:
        Break(): Statement(StatementType::Break) {}
    };
    class Continue : public Statement {
    public:
        Continue(): Statement(StatementType::Continue) {}
    };
    class Return : public Statement {
    public:
        Return(): Statement(StatementType::Return) {}
    };

    class Number : public Expression {
    public:
        const float number;
        Number(float num): Expression(ExpressionType::Number), number(num) {}
    };
    class String : public Expression {
    public:
        const std::string_view string;
        explicit String(const std::string_view str): Expression(ExpressionType::String), string(str) {}
    };
    class Boolean : public Expression {
    public:
        const bool boolean;
        Boolean(bool bool_): Expression(ExpressionType::Boolean), boolean(bool_) {}
    };
    class Object : public Expression {
    public:
        using values_type = const std::unordered_map<std::string_view, std::unique_ptr<Expression>>;
        values_type values;
        Object(std::remove_const_t<values_type> vals): Expression(ExpressionType::Object), values(std::move(vals)) {}
    };
    class Array : public Expression {
    public:
        const std::vector<std::unique_ptr<Expression>> values;
        Array(std::vector<std::unique_ptr<Expression>> vals): Expression(ExpressionType::Array), values(std::move(vals)) {}
    };
    class FuncCall : public Expression {
    public:
        const std::string_view name;
        const std::vector<std::unique_ptr<Expression>> args;
        FuncCall(std::string_view name_, std::vector<std::unique_ptr<Expression>> args_):
            Expression(ExpressionType::FuncCall), name(name_), args(std::move(args_)) {}
    };
    class IndexAccess : public Expression {
    public:
        const std::unique_ptr<Expression> target_expr;
        const std::unique_ptr<Expression> index_expr;
        IndexAccess(std::unique_ptr<Expression> t_expr, std::unique_ptr<Expression> i_expr):
            Expression(ExpressionType::IndexAccess), target_expr(std::move(t_expr)), index_expr(std::move(i_expr)) {}
    };
    class MemberAccess : public Expression {
    public:
        const std::unique_ptr<Expression> target_expr;
        const std::string_view member_name;
        MemberAccess(std::unique_ptr<Expression> t_expr, std::string_view m_name):
            Expression(ExpressionType::MemberAccess), target_expr(std::move(t_expr)), member_name(m_name) {}
    };

    class IndexReInit : public Statement {
    public:
        const std::unique_ptr<IndexAccess> target_expr;
        const std::unique_ptr<Expression> reinit_value;
        IndexReInit(std::unique_ptr<IndexAccess> t_expr, std::unique_ptr<Expression> value):
            Statement(StatementType::IndexReInit), target_expr(std::move(t_expr)), reinit_value(std::move(value)) {}
    };
    class MemberReInit : public Statement {
    public:
        const std::unique_ptr<MemberAccess> target_expr;
        const std::unique_ptr<Expression> reinit_value;
        MemberReInit(std::unique_ptr<MemberAccess> t_expr, std::unique_ptr<Expression> value):
            Statement(StatementType::IndexReInit), target_expr(std::move(t_expr)), reinit_value(std::move(value)) {}
    };

}
