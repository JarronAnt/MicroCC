#include "microcc/Lexer.hpp"

#include <cctype>
#include <charconv>
#include <stdexcept>
#include <string>
#include <system_error>

namespace microcc {

Lexer::Lexer(std::string_view source, std::string filename)
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
    if (isAtEnd()) {
        return '\0';
    }

    return source_[current_];
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

        switch (c) {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            advance();
            continue;

        default:
            break;
        }

        // Single-line comment:
        //
        //   // comment
        //
        if (c == '/' && peekNext() == '/') {
            advance();
            advance();

            while (!isAtEnd() && peek() != '\n') {
                advance();
            }

            continue;
        }

        // Block comment:
        //
        //   /* comment */
        //
        if (c == '/' && peekNext() == '*') {
            const SourceLocation location {
                current_,
                line_,
                column_
            };

            advance();
            advance();

            while (!isAtEnd()) {
                if (peek() == '*' && peekNext() == '/') {
                    advance();
                    advance();
                    break;
                }

                advance();
            }

            if (isAtEnd() &&
                !(current_ >= 2 &&
                  source_[current_ - 2] == '*' &&
                  source_[current_ - 1] == '/')) {
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
    return Token {
        .kind = kind,
        .lexeme = std::string(source_.substr(start, current_ - start)),
        .location = location,
        .integerValue = std::nullopt
    };
}

Token Lexer::lexIdentifierOrKeyword(
    std::size_t start,
    SourceLocation location
)
{
    while (!isAtEnd()) {
        const unsigned char c =
            static_cast<unsigned char>(peek());

        if (!std::isalnum(c) && c != '_') {
            break;
        }

        advance();
    }

    const std::string_view text =
        source_.substr(start, current_ - start);

    TokenKind kind = TokenKind::Identifier;

    if (text == "int") {
        kind = TokenKind::KwInt;
    } else if (text == "return") {
        kind = TokenKind::KwReturn;
    }

    return Token {
        .kind = kind,
        .lexeme = std::string(text),
        .location = location,
        .integerValue = std::nullopt
    };
}

Token Lexer::lexNumber(
    std::size_t start,
    SourceLocation location
)
{
    while (!isAtEnd()) {
        const unsigned char c =
            static_cast<unsigned char>(peek());

        if (!std::isdigit(c)) {
            break;
        }

        advance();
    }

    const std::string_view text =
        source_.substr(start, current_ - start);

    std::int64_t value = 0;

    const auto [ptr, ec] =
        std::from_chars(
            text.data(),
            text.data() + text.size(),
            value
        );

    if (ec != std::errc {} ||
        ptr != text.data() + text.size()) {
        error(location, "invalid integer literal");
    }

    return Token {
        .kind = TokenKind::IntegerLiteral,
        .lexeme = std::string(text),
        .location = location,
        .integerValue = value
    };
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        if (isAtEnd()) {
            tokens.push_back(Token {
                .kind = TokenKind::EndOfFile,
                .lexeme = "",
                .location = SourceLocation {
                    current_,
                    line_,
                    column_
                },
                .integerValue = std::nullopt
            });

            break;
        }

        const std::size_t start = current_;

        const SourceLocation location {
            current_,
            line_,
            column_
        };

        const char c = advance();

        const unsigned char uc =
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

        switch (c) {
        case '(':
            tokens.push_back(
                makeToken(
                    TokenKind::LeftParen,
                    start,
                    location
                )
            );
            break;

        case ')':
            tokens.push_back(
                makeToken(
                    TokenKind::RightParen,
                    start,
                    location
                )
            );
            break;

        case '{':
            tokens.push_back(
                makeToken(
                    TokenKind::LeftBrace,
                    start,
                    location
                )
            );
            break;

        case '}':
            tokens.push_back(
                makeToken(
                    TokenKind::RightBrace,
                    start,
                    location
                )
            );
            break;

        case ';':
            tokens.push_back(
                makeToken(
                    TokenKind::Semicolon,
                    start,
                    location
                )
            );
            break;

        default:
            error(
                location,
                std::string("unexpected character '") +
                    c +
                    "'"
            );
        }
    }

    return tokens;
}

void Lexer::error(
    SourceLocation location,
    std::string_view message
) const
{
    throw std::runtime_error(
        filename_ +
        ":" +
        std::to_string(location.line) +
        ":" +
        std::to_string(location.column) +
        ": error: " +
        std::string(message)
    );
}

} // namespace microcc
