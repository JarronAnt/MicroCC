#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace microcc {

struct SourceLocation {
    std::size_t offset = 0;
    std::size_t line = 1;
    std::size_t column = 1;
};

enum class TokenKind {
    EndOfFile,

    // Literals / identifiers
    Identifier,
    IntegerLiteral,

    // Keywords
    KwInt,
    KwReturn,

    // Punctuation
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    SourceLocation location;

    std::optional<std::int64_t> integerValue;
};

inline std::string_view tokenKindName(TokenKind kind)
{
    switch (kind) {
    case TokenKind::EndOfFile:
        return "EndOfFile";

    case TokenKind::Identifier:
        return "Identifier";

    case TokenKind::IntegerLiteral:
        return "IntegerLiteral";

    case TokenKind::KwInt:
        return "KwInt";

    case TokenKind::KwReturn:
        return "KwReturn";

    case TokenKind::LeftParen:
        return "LeftParen";

    case TokenKind::RightParen:
        return "RightParen";

    case TokenKind::LeftBrace:
        return "LeftBrace";

    case TokenKind::RightBrace:
        return "RightBrace";

    case TokenKind::Semicolon:
        return "Semicolon";
    }

    return "Unknown";
}

} // namespace microcc
