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
    Percent,

    // Relational
    Less,
    LessEqual,
    Greater,
    GreaterEqual,

    // Equality
    EqualEqual,
    BangEqual,

    // Bitwise
    Ampersand,
    Pipe,
    Caret,
    Tilde,

    // Logical
    AmpAmp,
    PipePipe,
    Bang,

    // Shifts
    ShiftLeft,
    ShiftRight,

    // Conditional and sequencing
    Question,
    Colon,
    Comma
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
    case TokenKind::Percent:        return "Percent";

    case TokenKind::Less:           return "Less";
    case TokenKind::LessEqual:      return "LessEqual";
    case TokenKind::Greater:        return "Greater";
    case TokenKind::GreaterEqual:   return "GreaterEqual";

    case TokenKind::EqualEqual:     return "EqualEqual";
    case TokenKind::BangEqual:      return "BangEqual";

    case TokenKind::Ampersand:      return "Ampersand";
    case TokenKind::Pipe:           return "Pipe";
    case TokenKind::Caret:          return "Caret";
    case TokenKind::Tilde:          return "Tilde";

    case TokenKind::AmpAmp:         return "AmpAmp";
    case TokenKind::PipePipe:       return "PipePipe";
    case TokenKind::Bang:           return "Bang";

    case TokenKind::ShiftLeft:      return "ShiftLeft";
    case TokenKind::ShiftRight:     return "ShiftRight";

    case TokenKind::Question:       return "Question";
    case TokenKind::Colon:          return "Colon";
    case TokenKind::Comma:          return "Comma";

    }

    return "Unknown";
}

} // namespace microcc
