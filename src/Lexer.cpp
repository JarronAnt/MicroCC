
#include "microcc/Lexer.hpp"

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace microcc {

Lexer::Lexer(std::string_view source, std::string filename)
    : source_(source), filename_(std::move(filename)) {}

bool Lexer::isAtEnd() const
{
    return current_ >= source_.size();
}

char Lexer::peek() const
{
    return isAtEnd() ? '\0' : source_[current_];
}

char Lexer::peekNext() const
{
    return current_ + 1 >= source_.size()
        ? '\0'
        : source_[current_ + 1];
}

char Lexer::advance()
{
    if (isAtEnd()) return '\0';

    const char c = source_[current_++];

    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }

    return c;
}

void Lexer::skipWhitespaceAndComments()
{
    while (!isAtEnd()) {
        const char c = peek();

        if (c == ' ' || c == '\t' ||
            c == '\r' || c == '\n') {
            advance();
            continue;
        }

        // Single-line comment
        if (c == '/' && peekNext() == '/') {
            advance();
            advance();

            while (!isAtEnd() && peek() != '\n')
                advance();

            continue;
        }

        // Block comment
        if (c == '/' && peekNext() == '*') {
            const SourceLocation location{
                current_,
                line_,
                column_
            };

            advance();
            advance();

            bool closed = false;

            while (!isAtEnd()) {
                if (peek() == '*' && peekNext() == '/') {
                    advance();
                    advance();
                    closed = true;
                    break;
                }

                advance();
            }

            if (!closed)
                error(location, "unterminated block comment");

            continue;
        }

        break;
    }
}

Token Lexer::makeToken(
    TokenKind kind,
    std::size_t start,
    SourceLocation location
) const
{
    return Token{
        kind,
        std::string(
            source_.substr(start, current_ - start)
        ),
        location,
        std::nullopt
    };
}

Token Lexer::lexIdentifierOrKeyword(
    std::size_t start,
    SourceLocation location
)
{
    while (!isAtEnd()) {
        const auto c =
            static_cast<unsigned char>(peek());

        if (!std::isalnum(c) && c != '_')
            break;

        advance();
    }

    const auto text =
        source_.substr(start, current_ - start);

    TokenKind kind = TokenKind::Identifier;

    if (text == "int") kind = TokenKind::KwInt;
    if (text == "return") kind = TokenKind::KwReturn;
    if (text == "if") kind = TokenKind::KwIf;
    if (text == "else") kind = TokenKind::KwElse;
    if (text == "while") kind = TokenKind::KwWhile;
    if (text == "do") kind = TokenKind::KwDo;
    if (text == "for") kind = TokenKind::KwFor;
    if (text == "break") kind = TokenKind::KwBreak;
    if (text == "continue") kind = TokenKind::KwContinue;

    return Token{
        kind,
        std::string(text),
        location,
        std::nullopt
    };
}

Token Lexer::lexNumber(
    std::size_t start,
    SourceLocation location
)
{
    while (
        !isAtEnd() &&
        std::isdigit(
            static_cast<unsigned char>(peek())
        )
    ) {
        advance();
    }

    const auto text =
        source_.substr(start, current_ - start);

    std::int64_t value = 0;

    const auto [ptr, ec] =
        std::from_chars(
            text.data(),
            text.data() + text.size(),
            value
        );

    if (
        ec != std::errc{} ||
        ptr != text.data() + text.size()
    ) {
        error(location, "invalid integer literal");
    }

    return Token{
        TokenKind::IntegerLiteral,
        std::string(text),
        location,
        value
    };
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        if (isAtEnd()) {
            tokens.push_back(Token{
                TokenKind::EndOfFile,
                "",
                {current_, line_, column_},
                std::nullopt
            });

            break;
        }

        const std::size_t start = current_;

        const SourceLocation location{
            current_,
            line_,
            column_
        };

        const char c = advance();

        const auto uc =
            static_cast<unsigned char>(c);

        if (std::isalpha(uc) || c == '_') {
            tokens.push_back(
                lexIdentifierOrKeyword(start, location)
            );
            continue;
        }

        if (std::isdigit(uc)) {
            tokens.push_back(
                lexNumber(start, location)
            );
            continue;
        }

        TokenKind kind;

        switch (c) {
        // Punctuation
        case '(':
            kind = TokenKind::LeftParen;
            break;
        case ')':
            kind = TokenKind::RightParen;
            break;
        case '{':
            kind = TokenKind::LeftBrace;
            break;
        case '}':
            kind = TokenKind::RightBrace;
            break;
        case ';':
            kind = TokenKind::Semicolon;
            break;

        // Arithmetic and compound assignment
        case '+':
            if (peek() == '+') {
                advance();
                kind = TokenKind::PlusPlus;
            } else if (peek() == '=') {
                advance();
                kind = TokenKind::PlusEqual;
            } else {
                kind = TokenKind::Plus;
            }
            break;

        case '-':
            if (peek() == '-') {
                advance();
                kind = TokenKind::MinusMinus;
            } else if (peek() == '=') {
                advance();
                kind = TokenKind::MinusEqual;
            } else {
                kind = TokenKind::Minus;
            }
            break;

        case '*':
            if (peek() == '=') {
                advance();
                kind = TokenKind::StarEqual;
            } else {
                kind = TokenKind::Star;
            }
            break;

        case '/':
            if (peek() == '=') {
                advance();
                kind = TokenKind::SlashEqual;
            } else {
                kind = TokenKind::Slash;
            }
            break;

        case '%':
            if (peek() == '=') {
                advance();
                kind = TokenKind::PercentEqual;
            } else {
                kind = TokenKind::Percent;
            }
            break;

        // Bitwise operators
        case '^':
            if (peek() == '=') {
                advance();
                kind = TokenKind::CaretEqual;
            } else {
                kind = TokenKind::Caret;
            }
            break;

        case '~':
            kind = TokenKind::Tilde;
            break;

        case '&':
            if (peek() == '&') {
                advance();
                kind = TokenKind::AmpAmp;
            } else if (peek() == '=') {
                advance();
                kind = TokenKind::AmpEqual;
            } else {
                kind = TokenKind::Ampersand;
            }
            break;

        case '|':
            if (peek() == '|') {
                advance();
                kind = TokenKind::PipePipe;
            } else if (peek() == '=') {
                advance();
                kind = TokenKind::PipeEqual;
            } else {
                kind = TokenKind::Pipe;
            }
            break;

        // Relational and shift operators
        case '<':
            if (peek() == '=') {
                advance();
                kind = TokenKind::LessEqual;
            } else if (peek() == '<') {
                advance();
                if (peek() == '=') {
                    advance();
                    kind = TokenKind::ShiftLeftEqual;
                } else {
                    kind = TokenKind::ShiftLeft;
                }
            } else {
                kind = TokenKind::Less;
            }
            break;

        case '>':
            if (peek() == '=') {
                advance();
                kind = TokenKind::GreaterEqual;
            } else if (peek() == '>') {
                advance();
                if (peek() == '=') {
                    advance();
                    kind = TokenKind::ShiftRightEqual;
                } else {
                    kind = TokenKind::ShiftRight;
                }
            } else {
                kind = TokenKind::Greater;
            }
            break;

        // Assignment and equality
        case '=':
            if (peek() == '=') {
                advance();
                kind = TokenKind::EqualEqual;
            } else {
                kind = TokenKind::Equal;
            }
            break;

        case '!':
            if (peek() == '=') {
                advance();
                kind = TokenKind::BangEqual;
            } else {
                kind = TokenKind::Bang;
            }
            break;

        // Conditional and comma
        case '?':
            kind = TokenKind::Question;
            break;

        case ':':
            kind = TokenKind::Colon;
            break;

        case ',':
            kind = TokenKind::Comma;
            break;

        default:
            error(
                location,
                std::string("unexpected character '")
                    + c + "'"
            );
        }

        tokens.push_back(
            makeToken(kind, start, location)
        );
    }

    return tokens;
}

void Lexer::error(
    SourceLocation location,
    std::string_view message
) const
{
    throw std::runtime_error(
        filename_ + ":" +
        std::to_string(location.line) + ":" +
        std::to_string(location.column) +
        ": error: " +
        std::string(message)
    );
}

} // namespace microcc

