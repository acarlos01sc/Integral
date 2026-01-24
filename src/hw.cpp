#include <iostream>
#include <string>
#include <vector>

#include "lib/ast.h"
#include "lib/lexer.h"
#include "lib/parser.h"
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

inline void printAST(const ast::Expr* expr, int indent = 0) {
    auto pad = [indent]() {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
    };

    if (auto n = dynamic_cast<const ast::NumberExpr*>(expr)) {
        pad();
        std::cout << "Number(" << n->value << ")\n";
    } else if (auto v = dynamic_cast<const ast::VariableExpr*>(expr)) {
        pad();
        std::cout << "Variable(" << v->name << ")\n";
    } else if (auto u = dynamic_cast<const ast::UnaryExpr*>(expr)) {
        pad();
        std::cout << "Unary(" << (u->op == ast::UnaryOp::Minus ? "-" : "+")
                  << ")\n";
        printAST(u->operand.get(), indent + 1);
    } else if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr)) {
        pad();
        std::cout << "Binary(";
        switch (b->op) {
            case ast::BinaryOp::Add:
                std::cout << "+";
                break;
            case ast::BinaryOp::Sub:
                std::cout << "-";
                break;
            case ast::BinaryOp::Mul:
                std::cout << "*";
                break;
            case ast::BinaryOp::Div:
                std::cout << "/";
                break;
            case ast::BinaryOp::Pow:
                std::cout << "^";
                break;
            default:
                std::cout << "?";
                break;
        }
        std::cout << ")\n";

        printAST(b->left.get(), indent + 1);
        printAST(b->right.get(), indent + 1);
    } else if (auto c = dynamic_cast<const ast::CallExpr*>(expr)) {
        pad();
        std::cout << "Call(" << c->callee << ")\n";
        printAST(c->argument.get(), indent + 1);
    }
}

int main(void) {
    std::string input = "";
    std::string stream;

    while (std::getline(std::cin, stream)) {
        input += stream;
    }

    Lexer lexer(input);
    /*
        std::vector<Token> tokens = lexer.tokenize();

        for (const Token& t : tokens) {
            std::cout << token_type_to_string(t.type) << " " << "'" << t.lexeme
                      << "'" << '\n';
        }
    */
    try {
        Parser parser(lexer);
        auto ast = parser.parse();
        std::cout << "AST:\n";
        printAST(ast.get());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}