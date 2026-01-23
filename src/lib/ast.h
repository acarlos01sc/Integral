#pragma once

#include <memory>
#include <string>

namespace ast {

enum class UnaryOp { Plus, Minus };

enum class BinaryOp { Add, Sub, Mul, Div };

struct Expr {
    virtual ~Expr() = default;
};

struct NumberExpr : Expr {
    double value;
    explicit NumberExpr(double value) : value(value) {}
};

struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string name) : name(std::move(name)) {}
};

struct UnaryExpr : Expr {
    UnaryOp op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand)
        : op(op), operand(std::move(operand)) {}
};

struct BinaryExpr : Expr {
    BinaryOp op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> left,
               std::unique_ptr<Expr> right)
        : op(op), left(std::move(left)), right(std::move(right)) {}
};

}  // namespace ast