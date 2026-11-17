
#include "microcc/Parser.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace microcc {

// ============================================================
// Constructor and token helpers
// ============================================================

Parser::Parser(
    std::vector<Token> tokens,
    std::string filename
)
    : tokens_(std::move(tokens)),
      filename_(std::move(filename)) {}

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
    if (!check(kind))
        return false;

    advance();
    return true;
}

const Token& Parser::advance()
{
    if (!isAtEnd())
        ++currentIndex_;

    return previous();
}

const Token& Parser::consume(
    TokenKind kind,
    std::string_view message
)
{
    if (check(kind))
        return advance();

    error(current(), message);
}

// ============================================================
// Symbol tables and lexical scopes
// ============================================================

void Parser::beginScope()
{
    scopes_.emplace_back();
}

void Parser::endScope()
{
    scopes_.pop_back();
}

std::size_t Parser::declareLocal(const Token& name)
{
    auto& scope = scopes_.back();

    if (scope.contains(name.lexeme)) {
        error(
            name,
            "redeclaration of '" + name.lexeme + "'"
        );
    }

    // Each local int receives an 8-byte stack slot.
    nextLocalOffset_ += 8;

    scope.emplace(
        name.lexeme,
        nextLocalOffset_
    );

    return nextLocalOffset_;
}

std::size_t Parser::lookupLocal(const Token& name) const
{
    // Search from the innermost scope outward.
    for (
        auto it = scopes_.rbegin();
        it != scopes_.rend();
        ++it
    ) {
        auto found = it->find(name.lexeme);

        if (found != it->end())
            return found->second;
    }

    error(
        name,
        "undeclared identifier '" + name.lexeme + "'"
    );
}

// ============================================================
// Translation unit and functions
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

    // A new function has its own local variables.
    nextLocalOffset_ = 0;
    loopDepth_ = 0;
    beginScope();

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
            check(TokenKind::KwInt)
                ? parseDeclaration()
                : parseStatement()
        );
    }

    consume(
        TokenKind::RightBrace,
        "expected '}' after function body"
    );

    endScope();

    // Round the total local storage up to 16 bytes.
    function->stackSize =
        (nextLocalOffset_ + 15) & ~std::size_t{15};

    return function;
}

// ============================================================
// Statements
// ============================================================

std::unique_ptr<Stmt> Parser::parseStatement()
{
    if (check(TokenKind::LeftBrace))
        return parseBlock();

    if (check(TokenKind::KwReturn))
        return parseReturnStatement();

    if (check(TokenKind::KwIf))
        return parseIfStatement();

    if (check(TokenKind::KwWhile))
        return parseWhileStatement();

    if (check(TokenKind::KwDo))
        return parseDoWhileStatement();

    if (check(TokenKind::KwFor))
        return parseForStatement();

    if (match(TokenKind::KwBreak)) {
        if (loopDepth_ == 0) {
            error(
                previous(),
                "'break' outside a loop"
            );
        }

        consume(
            TokenKind::Semicolon,
            "expected ';' after break"
        );

        return std::make_unique<BreakStmt>();
    }

    if (match(TokenKind::KwContinue)) {
        if (loopDepth_ == 0) {
            error(
                previous(),
                "'continue' outside a loop"
            );
        }

        consume(
            TokenKind::Semicolon,
            "expected ';' after continue"
        );

        return std::make_unique<ContinueStmt>();
    }

    // Empty statement: ;
    if (match(TokenKind::Semicolon))
        return std::make_unique<EmptyStmt>();

    // Expression statement: x += 1;
    auto expr = parseExpression();

    consume(
        TokenKind::Semicolon,
        "expected ';' after expression"
    );

    return std::make_unique<ExprStmt>(
        std::move(expr)
    );
}

// ============================================================
// Blocks and declarations
// ============================================================

std::unique_ptr<Stmt> Parser::parseBlock()
{
    consume(
        TokenKind::LeftBrace,
        "expected '{'"
    );

    beginScope();

    auto block = std::make_unique<BlockStmt>();

    while (!check(TokenKind::RightBrace)) {
        if (isAtEnd())
            error(current(), "expected '}' after block");

        block->statements.push_back(
            check(TokenKind::KwInt)
                ? parseDeclaration()
                : parseStatement()
        );
    }

    consume(
        TokenKind::RightBrace,
        "expected '}' after block"
    );

    endScope();

    return block;
}

std::unique_ptr<Stmt> Parser::parseDeclaration()
{
    consume(
        TokenKind::KwInt,
        "expected 'int'"
    );

    auto decl = std::make_unique<DeclStmt>();

    // Support int a = 1, b = 2, c;
    do {
        const Token& name = consume(
            TokenKind::Identifier,
            "expected variable name"
        );

        const auto offset = declareLocal(name);

        VarDecl var{
            name.lexeme,
            offset,
            nullptr
        };

        // Use parseAssignment(), not parseExpression(),
        // so commas separate declarators rather than
        // becoming comma operators.
        if (match(TokenKind::Equal)) {
            var.initializer = parseAssignment();
        }

        decl->variables.push_back(
            std::move(var)
        );

    } while (match(TokenKind::Comma));

    consume(
        TokenKind::Semicolon,
        "expected ';' after declaration"
    );

    return decl;
}

// ============================================================
// Return
// ============================================================

std::unique_ptr<Stmt> Parser::parseReturnStatement()
{
    consume(
        TokenKind::KwReturn,
        "expected 'return'"
    );

    auto expr = parseExpression();

    consume(
        TokenKind::Semicolon,
        "expected ';' after return expression"
    );

    return std::make_unique<ReturnStmt>(
        std::move(expr)
    );
}

// ============================================================
// If / Else
// ============================================================

std::unique_ptr<Stmt> Parser::parseIfStatement()
{
    consume(TokenKind::KwIf, "expected 'if'");

    consume(
        TokenKind::LeftParen,
        "expected '(' after if"
    );

    auto stmt = std::make_unique<IfStmt>();

    stmt->condition = parseExpression();

    consume(
        TokenKind::RightParen,
        "expected ')' after if condition"
    );

    stmt->thenBranch = parseStatement();

    // Naturally attaches else to the nearest if.
    if (match(TokenKind::KwElse)) {
        stmt->elseBranch = parseStatement();
    }

    return stmt;
}

// ============================================================
// While
// ============================================================

std::unique_ptr<Stmt> Parser::parseWhileStatement()
{
    consume(TokenKind::KwWhile, "expected 'while'");

    consume(
        TokenKind::LeftParen,
        "expected '(' after while"
    );

    auto stmt = std::make_unique<WhileStmt>();

    stmt->condition = parseExpression();

    consume(
        TokenKind::RightParen,
        "expected ')' after while condition"
    );

    ++loopDepth_;
    stmt->body = parseStatement();
    --loopDepth_;

    return stmt;
}

// ============================================================
// Do-While
// ============================================================

std::unique_ptr<Stmt> Parser::parseDoWhileStatement()
{
    consume(TokenKind::KwDo, "expected 'do'");

    auto stmt = std::make_unique<DoWhileStmt>();

    ++loopDepth_;
    stmt->body = parseStatement();
    --loopDepth_;

    consume(
        TokenKind::KwWhile,
        "expected 'while' after do body"
    );

    consume(
        TokenKind::LeftParen,
        "expected '(' after while"
    );

    stmt->condition = parseExpression();

    consume(
        TokenKind::RightParen,
        "expected ')' after do-while condition"
    );

    consume(
        TokenKind::Semicolon,
        "expected ';' after do-while"
    );

    return stmt;
}

// ============================================================
// For
// ============================================================

std::unique_ptr<Stmt> Parser::parseForStatement()
{
    consume(TokenKind::KwFor, "expected 'for'");

    consume(
        TokenKind::LeftParen,
        "expected '(' after for"
    );

    // A for-loop initializer gets its own scope.
    beginScope();

    auto stmt = std::make_unique<ForStmt>();

    // Initializer:
    // for (int i = 0; ...)
    // for (i = 0; ...)
    // for (; ...)
    if (check(TokenKind::KwInt)) {
        stmt->initializer = parseDeclaration();
    } else if (!match(TokenKind::Semicolon)) {
        auto expr = parseExpression();

        consume(
            TokenKind::Semicolon,
            "expected ';' after for initializer"
        );

        stmt->initializer =
            std::make_unique<ExprStmt>(
                std::move(expr)
            );
    }

    // Condition
    if (!check(TokenKind::Semicolon)) {
        stmt->condition = parseExpression();
    }

    consume(
        TokenKind::Semicolon,
        "expected ';' after for condition"
    );

    // Increment expression
    if (!check(TokenKind::RightParen)) {
        stmt->increment = parseExpression();
    }

    consume(
        TokenKind::RightParen,
        "expected ')' after for clauses"
    );

    ++loopDepth_;
    stmt->body = parseStatement();
    --loopDepth_;

    endScope();

    return stmt;
}

// ============================================================
// Expressions: comma and assignment
// ============================================================

std::unique_ptr<Expr> Parser::parseExpression()
{
    return parseComma();
}

std::unique_ptr<Expr> Parser::parseComma()
{
    auto left = parseAssignment();

    while (match(TokenKind::Comma)) {
        auto right = parseAssignment();

        left = std::make_unique<BinaryExpr>(
            BinaryOperator::Comma,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

// Assignment is right-associative:
// a = b = 3
std::unique_ptr<Expr> Parser::parseAssignment()
{
    auto left = parseConditional();

    const TokenKind kind = current().kind;

    AssignOperator op;

    switch (kind) {
    case TokenKind::Equal:
        op = AssignOperator::Set;
        break;
    case TokenKind::PlusEqual:
        op = AssignOperator::Add;
        break;
    case TokenKind::MinusEqual:
        op = AssignOperator::Subtract;
        break;
    case TokenKind::StarEqual:
        op = AssignOperator::Multiply;
        break;
    case TokenKind::SlashEqual:
        op = AssignOperator::Divide;
        break;
    case TokenKind::PercentEqual:
        op = AssignOperator::Remainder;
        break;
    case TokenKind::AmpEqual:
        op = AssignOperator::BitwiseAnd;
        break;
    case TokenKind::PipeEqual:
        op = AssignOperator::BitwiseOr;
        break;
    case TokenKind::CaretEqual:
        op = AssignOperator::BitwiseXor;
        break;
    case TokenKind::ShiftLeftEqual:
        op = AssignOperator::ShiftLeft;
        break;
    case TokenKind::ShiftRightEqual:
        op = AssignOperator::ShiftRight;
        break;
    default:
        return left;
    }

    const Token& token = advance();

    // Phase 3 supports variables as assignment targets.
    const auto* target =
        dynamic_cast<const VariableExpr*>(left.get());

    if (!target) {
        error(
            token,
            "left side of assignment must be a variable"
        );
    }

    const auto offset = target->offset;

    auto right = parseAssignment();

    return std::make_unique<AssignExpr>(
        offset,
        op,
        std::move(right)
    );
}

// ============================================================
// Conditional
// ============================================================

std::unique_ptr<Expr> Parser::parseConditional()
{
    auto condition = parseLogicalOr();

    if (!match(TokenKind::Question))
        return condition;

    // Full C expression permitted between ? and :
    auto thenExpr = parseExpression();

    consume(
        TokenKind::Colon,
        "expected ':' in conditional expression"
    );

    auto elseExpr = parseConditional();

    return std::make_unique<ConditionalExpr>(
        std::move(condition),
        std::move(thenExpr),
        std::move(elseExpr)
    );
}

// ============================================================
// Logical OR / AND
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
// Bitwise expressions
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
// Equality / Relational
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

        left = std::make_unique<BinaryExpr>(
            op == TokenKind::EqualEqual
                ? BinaryOperator::Equal
                : BinaryOperator::NotEqual,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

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
// Shifts / Arithmetic
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

        left = std::make_unique<BinaryExpr>(
            op == TokenKind::ShiftLeft
                ? BinaryOperator::ShiftLeft
                : BinaryOperator::ShiftRight,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive()
{
    auto left = parseMultiplicative();

    while (
        check(TokenKind::Plus) ||
        check(TokenKind::Minus)
    ) {
        const TokenKind op = advance().kind;

        auto right = parseMultiplicative();

        left = std::make_unique<BinaryExpr>(
            op == TokenKind::Plus
                ? BinaryOperator::Add
                : BinaryOperator::Subtract,
            std::move(left),
            std::move(right)
        );
    }

    return left;
}

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
// Unary / Prefix / Postfix
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

    if (
        check(TokenKind::PlusPlus) ||
        check(TokenKind::MinusMinus)
    ) {
        const Token& op = advance();

        auto operand = parseUnary();

        const auto* var =
            dynamic_cast<const VariableExpr*>(operand.get());

        if (!var) {
            error(
                op,
                "increment/decrement requires a variable"
            );
        }

        return std::make_unique<UpdateExpr>(
            var->offset,
            op.kind == TokenKind::PlusPlus,
            true
        );
    }

    return parsePostfix();
}

std::unique_ptr<Expr> Parser::parsePostfix()
{
    auto expr = parsePrimary();

    while (
        check(TokenKind::PlusPlus) ||
        check(TokenKind::MinusMinus)
    ) {
        const Token& op = advance();

        const auto* var =
            dynamic_cast<const VariableExpr*>(expr.get());

        if (!var) {
            error(
                op,
                "increment/decrement requires a variable"
            );
        }

        expr = std::make_unique<UpdateExpr>(
            var->offset,
            op.kind == TokenKind::PlusPlus,
            false
        );
    }

    return expr;
}

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
                "Phase 3 only supports integer literals "
                "representable by a signed 32-bit int"
            );
        }

        return std::make_unique<IntegerLiteral>(
            *token.integerValue
        );
    }

    if (match(TokenKind::Identifier)) {
        const Token& name = previous();

        return std::make_unique<VariableExpr>(
            name.lexeme,
            lookupLocal(name)
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

