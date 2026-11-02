#pragma once

#include "microcc/Token.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace microcc {

class Lexer {
public:
    Lexer(std::string_view source, std::string filename);

    [[nodiscard]]
    std::vector<Token> tokenize();

private:
    [[nodiscard]]
    bool isAtEnd() const;

    [[nodiscard]]
    char peek() const;

    [[nodiscard]]
    char peekNext() const;

    char advance();

    void skipWhitespaceAndComments();

    [[nodiscard]]
    Token lexIdentifierOrKeyword(
        std::size_t start,
        SourceLocation location
    );

    [[nodiscard]]
    Token lexNumber(
        std::size_t start,
        SourceLocation location
    );

    [[nodiscard]]
    Token makeToken(
        TokenKind kind,
        std::size_t start,
        SourceLocation location
    ) const;

    [[noreturn]]
    void error(
        SourceLocation location,
        std::string_view message
    ) const;

private:
    std::string_view source_;
    std::string filename_;

    std::size_t current_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;
};

} // namespace microcc
