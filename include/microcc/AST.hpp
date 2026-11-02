#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace microcc {

struct Expr {
    virtual ~Expr() = default;
};

struct IntegerLiteral final : Expr {
    explicit IntegerLiteral(std::int64_t value)
        : value(value)
    {
    }

    std::int64_t value;
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct ReturnStmt final : Stmt {
    explicit ReturnStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression))
    {
    }

    std::unique_ptr<Expr> expression;
};

struct FunctionDecl {
    std::string name;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct Program {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
};

} // namespace microcc
