#include "lexer.h"

#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>

#include "token.h"

Lexer::Lexer(const std::string &input)
    : input_(input), pos_(0), line_(1), column_(1) {}

// Lexer::~Lexer() {}

char Lexer::peek() const {
    // TODO: Return character at current position without advancing
    if (pos_ >= input_.size()) return '\0';
    return input_[pos_];
}

char Lexer::get() {
    // TODO: Return character at current position and advance
    if (pos_ >= input_.size()) return '\0';

    char c = input_[pos_++];

    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }

    return c;
}

void Lexer::skip_white_space() {
    // TODO: Skip whitespace characters and update line/column tracking
    while (std::isspace(peek())) get();
}

Token Lexer::read_number() {
    // TODO: Parse and return a number token
    size_t start = pos_;
    size_t start_line = line_;
    size_t start_column = column_;

    bool has_dot = false;

    // TODO: Add regional configuration
    while (std::isdigit(peek()) || peek() == '.') {
        if (peek() == '.') {
            if (has_dot) break;
            has_dot = true;
        }
        get();
    }

    return Token{TokenType::Number, input_.substr(start, pos_ - start),
                 start_line, start_column};
}

Token Lexer::read_identifier() {
    // TODO: Parse and return an identifier token
    size_t start = pos_;
    size_t start_line = line_;
    size_t start_column = column_;

    get();

    while (std::isalnum(peek()) || peek() == '_') get();

    return Token{TokenType::Identifier, input_.substr(start, pos_ - start),
                 start_line, start_column};
}

Token Lexer::next() {
    skip_white_space();

    size_t start_line = line_;
    size_t start_column = column_;

    char c = peek();

    if (c == '\0') {
        return Token{TokenType::EndOfFile, "", start_line, start_column};
    }

    if (std::isdigit(c) || c == '.') return read_number();

    if (std::isalpha(c)) return read_identifier();

    get();

    switch (c) {
        case '+':
            return Token{TokenType::Plus, "+", start_line, start_column};
        case '-':
            return Token{TokenType::Minus, "-", start_line, start_column};
        case '*':
            return Token{TokenType::Star, "*", start_line, start_column};
        case '/':
            return Token{TokenType::Slash, "/", start_line, start_column};
        case '^':
            return Token{TokenType::Caret, "^", start_line, start_column};
        case '(':
            return Token{TokenType::LParen, "(", start_line, start_column};
        case ')':
            return Token{TokenType::RParen, ")", start_line, start_column};
        case '|':
            return Token{TokenType::Pipe,"|",start_line,start_column};
        default:
            return Token{TokenType::Invalid,std::string(1,c),start_line,start_column};
    }

    throw std::runtime_error("Undefined caracter '" + std::string(1, c) +
                             " in line " + std::to_string(start_line) +
                             ", column " + std::to_string(start_column));
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        Token t = next();
        tokens.push_back(t);
        if (t.type == TokenType::EndOfFile) break;
    }

    return tokens;
}