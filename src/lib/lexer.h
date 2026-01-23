// lexical reader

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "token.h"

class Lexer {
   public:
    explicit Lexer(const std::string &input);

    Token next();
    std::vector<Token> tokenize();

   private:
    char peek() const;
    char get();
    void skip_white_space();

    Token read_number();
    Token read_identifier();

    const std::string &input_;
    size_t pos_;
    size_t line_;
    size_t column_;
};