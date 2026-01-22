// lexical reader

#pragma once

#include "token.h"
#include <string>
#include <vector>

class Lexer {
  public:
    explicit Lexer(const std::string &input);

    ~Lexer();
    
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
    int line_;
    int column_;
};