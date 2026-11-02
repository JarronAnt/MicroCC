#pragma once

#include "microcc/AST.hpp"
#include "microcc/Token.hpp"

#include <memory>
#include <string>
#include <vector>

namespace microcc {

class Parser {
public:
    Parser(
        std::vector<Token> tokens,
        std::string filename
    );

    [[nodiscard]]
    Program parseProgram();

private:
    [[nodiscard]]
    const Token& current() const;

    [[nodiscard]]
    const Token& previous() const;

    [[nodiscard]]
    bool isAtEnd() const;

    [[nodiscard]]
    bool check(TokenKind kind) const;

    bool match(TokenKind kind);

    const Token& advance();

    const Token& consume(
        TokenKind kind,
        std::string_view message
    );

    [[nodiscard]]
    std::unique_ptr<FunctionDecl> parseFunction();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseStatement();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseReturnStatement();

    [[nodiscard]]
    std::unique_ptr<Expr> parseExpression();

    [[noreturn]]
    void error(
        const Token& token,
        std::string_view message
    ) const;

private:
    std::vector<Token> tokens_;
    std::string filename_;

    std::size_t currentIndex_ = 0;
};

} // namespace microcc
