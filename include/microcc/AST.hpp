
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

// Integer literals: 42, 100, 1234
struct IntegerLiteral final : Expr {
    explicit IntegerLiteral(std::int64_t value)
        : value(value)
    {
    }

    std::int64_t value;
};

// ============================================================
// Unary Expressions
// ============================================================

enum class UnaryOperator {
    Plus,
    Negate,
    LogicalNot,
    BitwiseNot
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

// ============================================================
// Binary Expressions
// ============================================================

enum class BinaryOperator {
    // Arithmetic
    Add,
    Subtract,
    Multiply,
    Divide,
    Remainder,

    // Shifts
    ShiftLeft,
    ShiftRight,

    // Relational
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    // Equality
    Equal,
    NotEqual,

    // Bitwise
    BitwiseAnd,
    BitwiseXor,
    BitwiseOr,

    // Logical
    LogicalAnd,
    LogicalOr,

    // Comma
    Comma
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
// Conditional Expression
//
// condition ? thenExpr : elseExpr
// ============================================================

struct ConditionalExpr final : Expr {
    ConditionalExpr(
        std::unique_ptr<Expr> condition,
        std::unique_ptr<Expr> thenExpr,
        std::unique_ptr<Expr> elseExpr
    )
        : condition(std::move(condition))
        , thenExpr(std::move(thenExpr))
        , elseExpr(std::move(elseExpr))
    {
    }

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> thenExpr;
    std::unique_ptr<Expr> elseExpr;
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
// Function Declarations
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

