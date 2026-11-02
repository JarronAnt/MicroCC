#include "microcc/Parser.hpp"

#include <limits>
#include <stdexcept>
#include <string>

namespace microcc {

Parser::Parser(
    std::vector<Token> tokens,
    std::string filename
)
    : tokens_(std::move(tokens))
    , filename_(std::move(filename))
{
}

const Token& Parser::current() const
{
    return tokens_[currentIndex_];
}

const Token& Parser::previous() const
{
    return tokens_[currentIndex_ - 1];
}

bool Parser::isAtEnd() const
{
    return current().kind == TokenKind::EndOfFile;
}

bool Parser::check(TokenKind kind) const
{
    if (isAtEnd()) {
        return kind == TokenKind::EndOfFile;
    }

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
        "expected 'int' at beginning of function declaration"
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

std::unique_ptr<Stmt> Parser::parseStatement()
{
    if (check(TokenKind::KwReturn)) {
        return parseReturnStatement();
    }

    error(
        current(),
        "expected statement"
    );
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

std::unique_ptr<Expr> Parser::parseExpression()
{
    if (match(TokenKind::IntegerLiteral)) {
        const Token& token = previous();

        if (!token.integerValue.has_value()) {
            error(
                token,
                "integer token contains no value"
            );
        }

        if (*token.integerValue >
            std::numeric_limits<std::int32_t>::max()) {
            error(
                token,
                "Phase 1 only supports integer literals "
                "representable by a signed 32-bit int"
            );
        }

        return std::make_unique<IntegerLiteral>(
            *token.integerValue
        );
    }

    error(
        current(),
        "expected expression"
    );
}

void Parser::error(
    const Token& token,
    std::string_view message
) const
{
    throw std::runtime_error(
        filename_ +
        ":" +
        std::to_string(token.location.line) +
        ":" +
        std::to_string(token.location.column) +
        ": error: " +
        std::string(message)
    );
}

} // namespace microcc
