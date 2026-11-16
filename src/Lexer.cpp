
#include "microcc/Lexer.hpp"

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace microcc {

Lexer::Lexer(
    std::string_view source,
    std::string filename
)
    : source_(source)
    , filename_(std::move(filename))
{
}

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
    if (current_ + 1 >= source_.size()) {
        return '\0';
    }

    return source_[current_ + 1];
}

char Lexer::advance()
{
    if (isAtEnd()) {
        return '\0';
    }

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

        // Single-line comments
        if (c == '/' && peekNext() == '/') {
            advance();
            advance();

            while (!isAtEnd() && peek() != '\n') {
                advance();
            }

            continue;
        }

        // Block comments
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

            if (!closed) {
                error(location, "unterminated block comment");
            }

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

        if (!std::isalnum(c) && c != '_') {
            break;
        }

        advance();
    }

    const auto text =
        source_.substr(start, current_ - start);

    TokenKind kind = TokenKind::Identifier;

    if (text == "int") {
        kind = TokenKind::KwInt;
    } else if (text == "return") {
        kind = TokenKind::KwReturn;
    }

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
    while (!isAtEnd() &&
           std::isdigit(
               static_cast<unsigned char>(peek())
           )) {
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

    if (ec != std::errc{} ||
        ptr != text.data() + text.size()) {
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

        // Arithmetic
        case '+':
            kind = TokenKind::Plus;
            break;

        case '-':
            kind = TokenKind::Minus;
            break;

        case '*':
            kind = TokenKind::Star;
            break;

        case '/':
            kind = TokenKind::Slash;
            break;

        case '%':
            kind = TokenKind::Percent;
            break;

        // Bitwise and conditional
        case '^':
            kind = TokenKind::Caret;
            break;

        case '~':
            kind = TokenKind::Tilde;
            break;

        case '?':
            kind = TokenKind::Question;
            break;

        case ':':
            kind = TokenKind::Colon;
            break;

        case ',':
            kind = TokenKind::Comma;
            break;

        // Comparison and shifts
        case '<':
            if (peek() == '=') {
                advance();
                kind = TokenKind::LessEqual;
            } else if (peek() == '<') {
                advance();
                kind = TokenKind::ShiftLeft;
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
                kind = TokenKind::ShiftRight;
            } else {
                kind = TokenKind::Greater;
            }
            break;

        // Equality
        case '=':
            if (peek() == '=') {
                advance();
                kind = TokenKind::EqualEqual;
            } else {
                error(
                    location,
                    "assignment '=' is not supported until Phase 3"
                );
            }
            break;

        // Logical NOT or inequality
        case '!':
            if (peek() == '=') {
                advance();
                kind = TokenKind::BangEqual;
            } else {
                kind = TokenKind::Bang;
            }
            break;

        // Bitwise AND or logical AND
        case '&':
            if (peek() == '&') {
                advance();
                kind = TokenKind::AmpAmp;
            } else {
                kind = TokenKind::Ampersand;
            }
            break;

        // Bitwise OR or logical OR
        case '|':
            if (peek() == '|') {
                advance();
                kind = TokenKind::PipePipe;
            } else {
                kind = TokenKind::Pipe;
            }
            break;

        default:
            error(
                location,
                std::string("unexpected character '") +
                    c + "'"
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

