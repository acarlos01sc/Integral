#include "parser.h"

#include <memory>
#include <stdexcept>
#include <string>
#include "ast.h"
#include "lexer.h"
#include "token.h"

Parser::Parser(Lexer& lexer): lexer(lexer) {
    advance();
}

std::unique_ptr<ast::Expr> Parser::parse() {
    auto expr=expression();
    if (current.type != TokenType::EndOfFile) {
        throw std::runtime_error("Unexpected token after expression");
    }

    return expr;
}

void Parser::advance() {
    current=lexer.next();
}

bool Parser::match(TokenType type) {
    if (current.type==type) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TokenType type, const char* errorMessage) {
    if (!match(type)) {
        throw std::runtime_error(errorMessage);
    }
}

static ast::BinaryOp toBinaryOp(TokenType type) {
    switch (type) {
        case TokenType::Plus:
            return ast::BinaryOp::Add;
        case TokenType::Minus:
            return ast::BinaryOp::Sub;
        case TokenType::Star:
            return ast::BinaryOp::Mul;
        case TokenType::Slash:
            return ast::BinaryOp::Div;
        default:
            throw std::runtime_error("Invalid binary operator");
    }
}

static ast::UnaryOp toUnaryOp(TokenType type) {
    switch (type) {
        case TokenType::Plus:
            return ast::UnaryOp::Plus;
        case TokenType::Minus:
            return ast::UnaryOp::Minus;
        default:
            throw std::runtime_error("Invalid unary operator");
    }
}

std::unique_ptr<ast::Expr> Parser::expression() {
    auto left=term();

    while (current.type==TokenType::Plus||current.type==TokenType::Minus) {
        TokenType opToken = current.type;
        advance();

        auto right = term();
        left=std::make_unique<ast::BinaryExpr>(toBinaryOp(opToken),std::move(left),std::move(right));
    }
    return left;
}

std::unique_ptr<ast::Expr> Parser::term() {
    auto left=factor();

    while (current.type==TokenType::Star||current.type==TokenType::Slash) {
        TokenType opToken = current.type;
        advance();

        auto right=factor();
        left=std::make_unique<ast::BinaryExpr>(toBinaryOp(opToken),std::move(left),std::move(right));
    }
    return left;
}

std::unique_ptr<ast::Expr> Parser::factor() {
    switch (current.type) {
        case TokenType::Number:
            return parseNumber();
        case TokenType::Identifier:
            return parseVariable();
        case TokenType::LParen:
            return parseParenExpr();
        case TokenType::Plus:
        case TokenType::Minus:
            return parseUnary();
        default:
            throw std::runtime_error("Invalid Expression.");
    }
}

std::unique_ptr<ast::Expr> Parser::parseNumber() {
    double value=std::stod(current.lexeme);
    advance();
    return std::make_unique<ast::NumberExpr>(value);
}

std::unique_ptr<ast::Expr> Parser::parseVariable() {
    std::string name = current.lexeme;
    advance();
    return std::make_unique<ast::VariableExpr>(std::move(name));
}

std::unique_ptr<ast::Expr> Parser::parseParenExpr() {
    expect(TokenType::LParen,"Expected '('");
    auto expr=expression();
    expect(TokenType::RParen,"Expected ')'");
    return expr;
}

std::unique_ptr<ast::Expr> Parser::parseUnary() {
    TokenType opToken=current.type;
    advance();
    auto operand=factor();
    return std::make_unique<ast::UnaryExpr>(toUnaryOp(opToken),std::move(operand));
}