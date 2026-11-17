
#pragma once

#include <cstddef>
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

struct IntegerLiteral final : Expr {
    explicit IntegerLiteral(std::int64_t value)
        : value(value) {}

    std::int64_t value;
};

// Offset measured in bytes below frame pointer x29.
struct VariableExpr final : Expr {
    VariableExpr(std::string name, std::size_t offset)
        : name(std::move(name)), offset(offset) {}

    std::string name;
    std::size_t offset;
};

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
        : op(op), operand(std::move(operand)) {}

    UnaryOperator op;
    std::unique_ptr<Expr> operand;
};

enum class BinaryOperator {
    Add,
    Subtract,
    Multiply,
    Divide,
    Remainder,

    ShiftLeft,
    ShiftRight,

    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,

    BitwiseAnd,
    BitwiseXor,
    BitwiseOr,

    LogicalAnd,
    LogicalOr,
    Comma
};

struct BinaryExpr final : Expr {
    BinaryExpr(
        BinaryOperator op,
        std::unique_ptr<Expr> left,
        std::unique_ptr<Expr> right
    )
        : op(op),
          left(std::move(left)),
          right(std::move(right)) {}

    BinaryOperator op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

struct ConditionalExpr final : Expr {
    ConditionalExpr(
        std::unique_ptr<Expr> condition,
        std::unique_ptr<Expr> thenExpr,
        std::unique_ptr<Expr> elseExpr
    )
        : condition(std::move(condition)),
          thenExpr(std::move(thenExpr)),
          elseExpr(std::move(elseExpr)) {}

    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> thenExpr;
    std::unique_ptr<Expr> elseExpr;
};

// ============================================================
// Assignment Expressions
// ============================================================

enum class AssignOperator {
    Set,
    Add,
    Subtract,
    Multiply,
    Divide,
    Remainder,

    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,

    ShiftLeft,
    ShiftRight
};

struct AssignExpr final : Expr {
    AssignExpr(
        std::size_t offset,
        AssignOperator op,
        std::unique_ptr<Expr> value
    )
        : offset(offset),
          op(op),
          value(std::move(value)) {}

    std::size_t offset;
    AssignOperator op;
    std::unique_ptr<Expr> value;
};

// Prefix/postfix ++ and --.
struct UpdateExpr final : Expr {
    UpdateExpr(
        std::size_t offset,
        bool increment,
        bool prefix
    )
        : offset(offset),
          increment(increment),
          prefix(prefix) {}

    std::size_t offset;
    bool increment;
    bool prefix;
};

// ============================================================
// Statements
// ============================================================

struct Stmt {
    virtual ~Stmt() = default;
};

struct ReturnStmt final : Stmt {
    explicit ReturnStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    std::unique_ptr<Expr> expression;
};

struct EmptyStmt final : Stmt {};

struct ExprStmt final : Stmt {
    explicit ExprStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    std::unique_ptr<Expr> expression;
};

// One declarator within an int declaration.
struct VarDecl {
    std::string name;
    std::size_t offset;
    std::unique_ptr<Expr> initializer;
};

struct DeclStmt final : Stmt {
    std::vector<VarDecl> variables;
};

struct BlockStmt final : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

struct IfStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
};

struct WhileStmt final : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
};

struct DoWhileStmt final : Stmt {
    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> condition;
};

struct ForStmt final : Stmt {
    // Declaration, expression statement, or absent.
    std::unique_ptr<Stmt> initializer;

    // Missing condition means true.
    std::unique_ptr<Expr> condition;

    // Optional increment expression.
    std::unique_ptr<Expr> increment;

    std::unique_ptr<Stmt> body;
};

struct BreakStmt final : Stmt {};
struct ContinueStmt final : Stmt {};

// ============================================================
// Functions and Translation Unit
// ============================================================

struct FunctionDecl {
    std::string name;
    std::vector<std::unique_ptr<Stmt>> body;

    // Local stack size, rounded to 16-byte alignment.
    std::size_t stackSize = 0;
};

struct Program {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
};

} // namespace microcc

