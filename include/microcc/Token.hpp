
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
    Identifier,
    IntegerLiteral,

    // Keywords
    KwInt,
    KwReturn,
    KwIf,
    KwElse,
    KwWhile,
    KwDo,
    KwFor,
    KwBreak,
    KwContinue,

    // Punctuation
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    Semicolon,

    // Arithmetic
    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    // Comparison and equality
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    EqualEqual,
    BangEqual,

    // Bitwise and logical
    Ampersand,
    Pipe,
    Caret,
    Tilde,
    AmpAmp,
    PipePipe,
    Bang,

    // Shifts and conditional
    ShiftLeft,
    ShiftRight,
    Question,
    Colon,
    Comma,

    // Assignment
    Equal,
    PlusEqual,
    MinusEqual,
    StarEqual,
    SlashEqual,
    PercentEqual,
    AmpEqual,
    PipeEqual,
    CaretEqual,
    ShiftLeftEqual,
    ShiftRightEqual,

    // Increment and decrement
    PlusPlus,
    MinusMinus
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
    case TokenKind::EndOfFile: return "EndOfFile";
    case TokenKind::Identifier: return "Identifier";
    case TokenKind::IntegerLiteral: return "IntegerLiteral";

    case TokenKind::KwInt: return "KwInt";
    case TokenKind::KwReturn: return "KwReturn";
    case TokenKind::KwIf: return "KwIf";
    case TokenKind::KwElse: return "KwElse";
    case TokenKind::KwWhile: return "KwWhile";
    case TokenKind::KwDo: return "KwDo";
    case TokenKind::KwFor: return "KwFor";
    case TokenKind::KwBreak: return "KwBreak";
    case TokenKind::KwContinue: return "KwContinue";

    case TokenKind::LeftParen: return "LeftParen";
    case TokenKind::RightParen: return "RightParen";
    case TokenKind::LeftBrace: return "LeftBrace";
    case TokenKind::RightBrace: return "RightBrace";
    case TokenKind::Semicolon: return "Semicolon";

    case TokenKind::Plus: return "Plus";
    case TokenKind::Minus: return "Minus";
    case TokenKind::Star: return "Star";
    case TokenKind::Slash: return "Slash";
    case TokenKind::Percent: return "Percent";

    case TokenKind::Less: return "Less";
    case TokenKind::LessEqual: return "LessEqual";
    case TokenKind::Greater: return "Greater";
    case TokenKind::GreaterEqual: return "GreaterEqual";
    case TokenKind::EqualEqual: return "EqualEqual";
    case TokenKind::BangEqual: return "BangEqual";

    case TokenKind::Ampersand: return "Ampersand";
    case TokenKind::Pipe: return "Pipe";
    case TokenKind::Caret: return "Caret";
    case TokenKind::Tilde: return "Tilde";
    case TokenKind::AmpAmp: return "AmpAmp";
    case TokenKind::PipePipe: return "PipePipe";
    case TokenKind::Bang: return "Bang";

    case TokenKind::ShiftLeft: return "ShiftLeft";
    case TokenKind::ShiftRight: return "ShiftRight";
    case TokenKind::Question: return "Question";
    case TokenKind::Colon: return "Colon";
    case TokenKind::Comma: return "Comma";

    case TokenKind::Equal: return "Equal";
    case TokenKind::PlusEqual: return "PlusEqual";
    case TokenKind::MinusEqual: return "MinusEqual";
    case TokenKind::StarEqual: return "StarEqual";
    case TokenKind::SlashEqual: return "SlashEqual";
    case TokenKind::PercentEqual: return "PercentEqual";
    case TokenKind::AmpEqual: return "AmpEqual";
    case TokenKind::PipeEqual: return "PipeEqual";
    case TokenKind::CaretEqual: return "CaretEqual";
    case TokenKind::ShiftLeftEqual: return "ShiftLeftEqual";
    case TokenKind::ShiftRightEqual: return "ShiftRightEqual";

    case TokenKind::PlusPlus: return "PlusPlus";
    case TokenKind::MinusMinus: return "MinusMinus";
    }

    return "Unknown";
}

} // namespace microcc

