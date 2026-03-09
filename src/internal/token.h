#pragma once

#include <cstddef>
#include <string>

// Token type
enum class TokenType {
    Number,
    Identifier,
    Plus,
    Minus,
    Star,
    Slash,
    Caret,
    Pipe,
    LParen,
    RParen,
    Comma,
    EndOfFile,
    Invalid
};

// Token
struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};