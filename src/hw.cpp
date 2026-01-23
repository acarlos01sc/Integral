#include <iostream>
#include <string>
#include <vector>

#include "lib/lexer.h"
#include "lib/token.h"

std::string token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::Number:
            return "Number";
        case TokenType::Identifier:
            return "Identifier";
        case TokenType::Plus:
            return "Plus";
        case TokenType::Minus:
            return "Minus";
        case TokenType::Star:
            return "Star";
        case TokenType::Slash:
            return "Slash";
        case TokenType::Caret:
            return "Caret";
        case TokenType::LParen:
            return "LParen";
        case TokenType::RParen:
            return "RParen";
        case TokenType::EndOfFile:
            return "EndOfFile";
        case TokenType::Invalid:
            return "Invalid";
    }

    return "Unknown";
}

int main(void) {
    std::string input = "sin(theta*x) + sqrt(v2*x^3) * PI/2";

    Lexer lexer(input);

    std::vector<Token> tokens = lexer.tokenize();

    for (const Token &t : tokens) {
        std::cout << token_type_to_string(t.type) << " " << "'" << t.lexeme
                  << "'" << '\n';
    }

    return 0;
}