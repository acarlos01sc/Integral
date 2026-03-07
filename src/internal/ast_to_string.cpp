#include "internal/ast_to_string.h"

#include <sstream>
#include <stdexcept>

namespace numathap::internal {

static std::string unary_op_to_string(ast::UnaryOp op) {
    switch (op) {
        case ast::UnaryOp::Plus: return "+";
        case ast::UnaryOp::Minus: return "-";
        case ast::UnaryOp::Abs: return "abs";
    }
    throw std::runtime_error("unknown unary operator");
}

static std::string binary_op_to_string(ast::BinaryOp op) {
    switch (op) {
        case ast::BinaryOp::Add: return "+";
        case ast::BinaryOp::Sub: return "-";
        case ast::BinaryOp::Mul: return "*";
        case ast::BinaryOp::Div: return "/";
        case ast::BinaryOp::Pow: return "^";
    }
    throw std::runtime_error("unknown binary operator");
}

std::string ast_to_string(const ast::Expr* expr) {

    if (auto n = dynamic_cast<const ast::NumberExpr*>(expr)) {
        std::ostringstream out;
        out << n->value;
        return out.str();
    }

    if (auto v = dynamic_cast<const ast::VariableExpr*>(expr)) {
        return v->name;
    }

    if (auto u = dynamic_cast<const ast::UnaryExpr*>(expr)) {
        if (u->op == ast::UnaryOp::Abs) {
            return "abs(" + ast_to_string(u->operand.get()) + ")";
        }

        return unary_op_to_string(u->op) +
               ast_to_string(u->operand.get());
    }

    if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr)) {

        return "(" +
               ast_to_string(b->left.get()) +
               binary_op_to_string(b->op) +
               ast_to_string(b->right.get()) +
               ")";
    }

    if (auto c = dynamic_cast<const ast::CallExpr*>(expr)) {

        return c->callee + "(" +
               ast_to_string(c->argument.get()) +
               ")";
    }

    throw std::runtime_error("unknown AST node");
}

}