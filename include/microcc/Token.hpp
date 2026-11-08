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

    // Literals and identifiers
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

    // Arithmetic operators
    Plus,
    Minus,
    Star,
    Slash,
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
    case TokenKind::EndOfFile:     return "EndOfFile";
    case TokenKind::Identifier:    return "Identifier";
    case TokenKind::IntegerLiteral:return "IntegerLiteral";

    case TokenKind::KwInt:         return "KwInt";
    case TokenKind::KwReturn:      return "KwReturn";

    case TokenKind::LeftParen:     return "LeftParen";
    case TokenKind::RightParen:    return "RightParen";
    case TokenKind::LeftBrace:     return "LeftBrace";
    case TokenKind::RightBrace:    return "RightBrace";
    case TokenKind::Semicolon:     return "Semicolon";

    case TokenKind::Plus:          return "Plus";
    case TokenKind::Minus:         return "Minus";
    case TokenKind::Star:          return "Star";
    case TokenKind::Slash:         return "Slash";
    }

    return "Unknown";
}

} // namespace microcc
