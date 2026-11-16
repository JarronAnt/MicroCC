
#include "microcc/Parser.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace microcc {

// ============================================================
// Constructor
// ============================================================

Parser::Parser(
    std::vector<Token> tokens,
    std::string filename
)
    : tokens_(std::move(tokens))
    , filename_(std::move(filename))
{
}

// ============================================================
// Token helpers
// ============================================================

const Token& Parser::current() const
{
    return tokens_.at(currentIndex_);
}

const Token& Parser::previous() const
{
    return tokens_.at(currentIndex_ - 1);
}

bool Parser::isAtEnd() const
{
    return current().kind == TokenKind::EndOfFile;
}

bool Parser::check(TokenKind kind) const
{
    return current().kind == kind;
}

bool Parser::match(TokenKind kind)
{
    if (!check(kind)) {
        return false;
    }

    advance();
    return true;
}

const Token& Parser::advance()
{
    if (!isAtEnd()) {
        ++currentIndex_;
    }

    return previous();
}

const Token& Parser::consume(
    TokenKind kind,
    std::string_view message
)
{
    if (check(kind)) {
        return advance();
    }

    error(current(), message);
}

// ============================================================
// Program
// ============================================================

Program Parser::parseProgram()
{
    Program program;

    while (!isAtEnd()) {
        program.functions.push_back(
            parseFunction()
        );
    }

    return program;
}

// ============================================================
// Function declarations
// ============================================================

std::unique_ptr<FunctionDecl> Parser::parseFunction()
{
    consume(
        TokenKind::KwInt,
        "expected 'int' at beginning of function"
    );

    const Token& name = consume(
        TokenKind::Identifier,
        "expected function name"
    );

    consume(
        TokenKind::LeftParen,
        "expected '(' after function name"
    );

    consume(
        TokenKind::RightParen,
        "expected ')' after '('"
    );

    consume(
        TokenKind::LeftBrace,
        "expected '{' before function body"
    );

    auto function = std::make_unique<FunctionDecl>();
    function->name = name.lexeme;

    while (!check(TokenKind::RightBrace)) {
        if (isAtEnd()) {
            error(
                current(),
                "expected '}' after function body"
            );
        }

        function->body.push_back(
            parseStatement()
        );
    }

    consume(
        TokenKind::RightBrace,
        "expected '}' after function body"
    );

    return function;
}

// ============================================================
// Statements
// ============================================================

std::unique_ptr<Stmt> Parser::parseStatement()
{
    if (check(TokenKind::KwReturn)) {
        return parseReturnStatement();
    }

    error(current(), "expected statement");
}

std::unique_ptr<Stmt> Parser::parseReturnStatement()
{
    consume(
        TokenKind::KwReturn,
        "expected 'return'"
    );

    auto expression = parseExpression();

    consume(
        TokenKind::Semicolon,
        "expected ';' after return expression"
    );

    return std::make_unique<ReturnStmt>(
        std::move(expression)
    );
}

// ============================================================
// Expression entry point
// ============================================================

std::unique_ptr<Expr> Parser::parseExpression()
{
    return parseComma();
}

// ============================================================
// Comma operator
//
// a, b
//
// Lowest precedence.
// Evaluate a, discard its value, then evaluate b.
// ============================================================

std::unique_ptr<Expr> Parser::parseComma()
{
    auto left = parseConditional();

    while (match(TokenKind::Comma)) {
        auto right = parseConditional();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::Comma,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Conditional operator
//
// condition ? thenExpression : elseExpression
//
// Right-associative.
// Only one result branch will execute.
// ============================================================

std::unique_ptr<Expr> Parser::parseConditional()
{
    auto condition = parseLogicalOr();

    if (!match(TokenKind::Question)) {
        return condition;
    }

    // C permits a full expression (including comma)
    // between '?' and ':'.
    auto thenExpr = parseExpression();

    consume(
        TokenKind::Colon,
        "expected ':' in conditional expression"
    );

    // The else branch is another conditional expression.
    // This preserves right associativity.
    auto elseExpr = parseConditional();

    return std::make_unique<ConditionalExpr>(
        std::move(condition),
        std::move(thenExpr),
        std::move(elseExpr)
    );
}

// ============================================================
// Logical OR
//
// a || b
// ============================================================

std::unique_ptr<Expr> Parser::parseLogicalOr()
{
    auto left = parseLogicalAnd();

    while (match(TokenKind::PipePipe)) {
        auto right = parseLogicalAnd();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::LogicalOr,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Logical AND
//
// a && b
// ============================================================

std::unique_ptr<Expr> Parser::parseLogicalAnd()
{
    auto left = parseBitwiseOr();

    while (match(TokenKind::AmpAmp)) {
        auto right = parseBitwiseOr();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::LogicalAnd,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Bitwise OR
//
// a | b
// ============================================================

std::unique_ptr<Expr> Parser::parseBitwiseOr()
{
    auto left = parseBitwiseXor();

    while (match(TokenKind::Pipe)) {
        auto right = parseBitwiseXor();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::BitwiseOr,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Bitwise XOR
//
// a ^ b
// ============================================================

std::unique_ptr<Expr> Parser::parseBitwiseXor()
{
    auto left = parseBitwiseAnd();

    while (match(TokenKind::Caret)) {
        auto right = parseBitwiseAnd();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::BitwiseXor,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Bitwise AND
//
// a & b
// ============================================================

std::unique_ptr<Expr> Parser::parseBitwiseAnd()
{
    auto left = parseEquality();

    while (match(TokenKind::Ampersand)) {
        auto right = parseEquality();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::BitwiseAnd,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Equality
//
// == !=
// ============================================================

std::unique_ptr<Expr> Parser::parseEquality()
{
    auto left = parseRelational();

    while (
        check(TokenKind::EqualEqual) ||
        check(TokenKind::BangEqual)
    ) {
        const TokenKind op = advance().kind;

        auto right = parseRelational();

        const BinaryOperator binaryOp =
            op == TokenKind::EqualEqual
                ? BinaryOperator::Equal
                : BinaryOperator::NotEqual;

        left = std::make_unique<BinaryExpr>(
            binaryOp,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Relational operators
//
// < <= > >=
// ============================================================

std::unique_ptr<Expr> Parser::parseRelational()
{
    auto left = parseShift();

    while (
        check(TokenKind::Less) ||
        check(TokenKind::LessEqual) ||
        check(TokenKind::Greater) ||
        check(TokenKind::GreaterEqual)
    ) {
        const TokenKind op = advance().kind;

        auto right = parseShift();

        BinaryOperator binaryOp;

        switch (op) {
        case TokenKind::Less:
            binaryOp = BinaryOperator::Less;
            break;

        case TokenKind::LessEqual:
            binaryOp = BinaryOperator::LessEqual;
            break;

        case TokenKind::Greater:
            binaryOp = BinaryOperator::Greater;
            break;

        default:
            binaryOp = BinaryOperator::GreaterEqual;
            break;
        }

        left = std::make_unique<BinaryExpr>(
            binaryOp,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Bit shifts
//
// << >>
// ============================================================

std::unique_ptr<Expr> Parser::parseShift()
{
    auto left = parseAdditive();

    while (
        check(TokenKind::ShiftLeft) ||
        check(TokenKind::ShiftRight)
    ) {
        const TokenKind op = advance().kind;

        auto right = parseAdditive();

        const BinaryOperator binaryOp =
            op == TokenKind::ShiftLeft
                ? BinaryOperator::ShiftLeft
                : BinaryOperator::ShiftRight;

        left = std::make_unique<BinaryExpr>(
            binaryOp,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Addition and subtraction
//
// + -
// ============================================================

std::unique_ptr<Expr> Parser::parseAdditive()
{
    auto left = parseMultiplicative();

    while (
        check(TokenKind::Plus) ||
        check(TokenKind::Minus)
    ) {
        const TokenKind op = advance().kind;

        auto right = parseMultiplicative();

        const BinaryOperator binaryOp =
            op == TokenKind::Plus
                ? BinaryOperator::Add
                : BinaryOperator::Subtract;

        left = std::make_unique<BinaryExpr>(
            binaryOp,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Multiplication, division, remainder
//
// * / %
// ============================================================

std::unique_ptr<Expr> Parser::parseMultiplicative()
{
    auto left = parseUnary();

    while (
        check(TokenKind::Star) ||
        check(TokenKind::Slash) ||
        check(TokenKind::Percent)
    ) {
        const TokenKind op = advance().kind;

        auto right = parseUnary();

        BinaryOperator binaryOp;

        switch (op) {
        case TokenKind::Star:
            binaryOp = BinaryOperator::Multiply;
            break;

        case TokenKind::Slash:
            binaryOp = BinaryOperator::Divide;
            break;

        default:
            binaryOp = BinaryOperator::Remainder;
            break;
        }

        left = std::make_unique<BinaryExpr>(
            binaryOp,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// ============================================================
// Unary operators
//
// + - ! ~
// ============================================================

std::unique_ptr<Expr> Parser::parseUnary()
{
    if (match(TokenKind::Plus)) {
        return std::make_unique<UnaryExpr>(
            UnaryOperator::Plus,
            parseUnary()
        );
    }

    if (match(TokenKind::Minus)) {
        return std::make_unique<UnaryExpr>(
            UnaryOperator::Negate,
            parseUnary()
        );
    }

    if (match(TokenKind::Bang)) {
        return std::make_unique<UnaryExpr>(
            UnaryOperator::LogicalNot,
            parseUnary()
        );
    }

    if (match(TokenKind::Tilde)) {
        return std::make_unique<UnaryExpr>(
            UnaryOperator::BitwiseNot,
            parseUnary()
        );
    }

    return parsePrimary();
}

// ============================================================
// Primary expressions
//
// Integer literals and parentheses.
// ============================================================

std::unique_ptr<Expr> Parser::parsePrimary()
{
    if (match(TokenKind::IntegerLiteral)) {
        const Token& token = previous();

        if (!token.integerValue.has_value()) {
            error(
                token,
                "integer token contains no value"
            );
        }

        if (
            *token.integerValue >
            std::numeric_limits<std::int32_t>::max()
        ) {
            error(
                token,
                "Phase 2 only supports integer literals "
                "representable by a signed 32-bit int"
            );
        }

        return std::make_unique<IntegerLiteral>(
            *token.integerValue
        );
    }

    if (match(TokenKind::LeftParen)) {
        auto expression = parseExpression();

        consume(
            TokenKind::RightParen,
            "expected ')' after expression"
        );

        return expression;
    }

    error(current(), "expected expression");
}

// ============================================================
// Diagnostics
// ============================================================

void Parser::error(
    const Token& token,
    std::string_view message
) const
{
    throw std::runtime_error(
        filename_ + ":" +
        std::to_string(token.location.line) + ":" +
        std::to_string(token.location.column) +
        ": error: " +
        std::string(message)
    );
}

} // namespace microcc

