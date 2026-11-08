#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace microcc {

// ============================================================
// Expressions
// ============================================================

struct Expr {
    virtual ~Expr() = default;
};

// Integer literal: 42
struct IntegerLiteral final : Expr {
    explicit IntegerLiteral(std::int64_t value)
        : value(value)
    {
    }

    std::int64_t value;
};

// Unary operators: +x, -x
enum class UnaryOperator {
    Plus,
    Negate
};

struct UnaryExpr final : Expr {
    UnaryExpr(
        UnaryOperator op,
        std::unique_ptr<Expr> operand
    )
        : op(op)
        , operand(std::move(operand))
    {
    }

    UnaryOperator op;
    std::unique_ptr<Expr> operand;
};

// Binary operators: +, -, *, /
enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide
};

struct BinaryExpr final : Expr {
    BinaryExpr(
        BinaryOperator op,
        std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right
    )
        : op(op)
        , left(std::move(left))
        , right(std::move(right))
    {
    }

    BinaryOperator op;

    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

// ============================================================
// Statements
// ============================================================

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

// ============================================================
// Functions
// ============================================================

struct FunctionDecl {
    std::string name;
    std::vector<std::unique_ptr<Stmt>> body;
};

// ============================================================
// Translation Unit
// ============================================================

struct Program {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
};

} // namespace microcc
