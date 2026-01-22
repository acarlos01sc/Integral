#include "lexer.h"

Lexer::Lexer(const std::string &input) : input_(input), pos_(0), line_(1), column_(1) {}

Lexer::~Lexer() {}

char Lexer::peek() const {
    // TODO: Return character at current position without advancing
}

char Lexer::get() {
    // TODO: Return character at current position and advance
}

void Lexer::skip_white_space() {
    // TODO: Skip whitespace characters and update line/column tracking
}

Token Lexer::read_number() {
    // TODO: Parse and return a number token
}

Token Lexer::read_identifier() {
    // TODO: Parse and return an identifier token
}