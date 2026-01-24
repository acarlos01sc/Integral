#pragma once

#include <memory>
#include <vector>

#include "ast.h"
#include "lexer.h"
#include "token.h"

class Parser {
   public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<ast::Expr> parse();

   private:
    Lexer& lexer;
    Token current;

    void advance();
    bool match(TokenType type);
    void expect(TokenType type, const char* errorMessage);

    std::unique_ptr<ast::Expr> expression();
    std::unique_ptr<ast::Expr> term();
    std::unique_ptr<ast::Expr> factor();

    std::unique_ptr<ast::Expr> parseNumber();
    std::unique_ptr<ast::Expr> parseVariable();
    std::unique_ptr<ast::Expr> parseParenExpr();
    std::unique_ptr<ast::Expr> parseUnary();
    std::unique_ptr<ast::Expr> parsePower();
};
