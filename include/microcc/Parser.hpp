
#pragma once

#include "microcc/AST.hpp"
#include "microcc/Token.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
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
    // Token helpers
    [[nodiscard]] const Token& current() const;
    [[nodiscard]] const Token& previous() const;
    [[nodiscard]] bool isAtEnd() const;
    [[nodiscard]] bool check(TokenKind kind) const;

    bool match(TokenKind kind);
    const Token& advance();

    const Token& consume(
        TokenKind kind,
        std::string_view message
    );

    // Symbol tables and scopes
    void beginScope();
    void endScope();

    std::size_t declareLocal(const Token& name);

    [[nodiscard]]
    std::size_t lookupLocal(const Token& name) const;

    // Functions and statements
    [[nodiscard]]
    std::unique_ptr<FunctionDecl> parseFunction();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseStatement();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseBlock();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseDeclaration();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseReturnStatement();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseIfStatement();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseWhileStatement();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseDoWhileStatement();

    [[nodiscard]]
    std::unique_ptr<Stmt> parseForStatement();

    // Expression parsing
    [[nodiscard]]
    std::unique_ptr<Expr> parseExpression();

    [[nodiscard]]
    std::unique_ptr<Expr> parseComma();

    [[nodiscard]]
    std::unique_ptr<Expr> parseAssignment();

    [[nodiscard]]
    std::unique_ptr<Expr> parseConditional();

    [[nodiscard]]
    std::unique_ptr<Expr> parseLogicalOr();

    [[nodiscard]]
    std::unique_ptr<Expr> parseLogicalAnd();

    [[nodiscard]]
    std::unique_ptr<Expr> parseBitwiseOr();

    [[nodiscard]]
    std::unique_ptr<Expr> parseBitwiseXor();

    [[nodiscard]]
    std::unique_ptr<Expr> parseBitwiseAnd();

    [[nodiscard]]
    std::unique_ptr<Expr> parseEquality();

    [[nodiscard]]
    std::unique_ptr<Expr> parseRelational();

    [[nodiscard]]
    std::unique_ptr<Expr> parseShift();

    [[nodiscard]]
    std::unique_ptr<Expr> parseAdditive();

    [[nodiscard]]
    std::unique_ptr<Expr> parseMultiplicative();

    [[nodiscard]]
    std::unique_ptr<Expr> parseUnary();

    [[nodiscard]]
    std::unique_ptr<Expr> parsePostfix();

    [[nodiscard]]
    std::unique_ptr<Expr> parsePrimary();

    // Diagnostics
    [[noreturn]]
    void error(
        const Token& token,
        std::string_view message
    ) const;

private:
    std::vector<Token> tokens_;
    std::string filename_;

    std::size_t currentIndex_ = 0;

    std::vector<
        std::unordered_map<std::string, std::size_t>
    > scopes_;

    std::size_t nextLocalOffset_ = 0;
    unsigned loopDepth_ = 0;
};

} // namespace microcc

