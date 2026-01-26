#include "evaluator.h"

#include <cmath>
#include <stdexcept>
#include "ast.h"

double Evaluator::evaluate(const ast::Expr& expr,const Context& ctx) {
    if (auto num=dynamic_cast<const ast::NumberExpr*>(&expr))
        return evalNumber(*num);
    if (auto var=dynamic_cast<const ast::VariableExpr*>(&expr))
        return evalVariable(*var, ctx);
    if (auto un=dynamic_cast<const ast::UnaryExpr*>(&expr))
        return evalUnary(*un, ctx);
    if (auto bin=dynamic_cast<const ast::BinaryExpr*>(&expr))
        return evalBinary(*bin, ctx);

    throw std::runtime_error("Undefined expression type");
}

double Evaluator::evalNumber(const ast::NumberExpr& expr) {
    return expr.value;
}

double Evaluator::evalVariable(const ast::VariableExpr& expr,const Context& ctx) {
    auto it=ctx.find(expr.name);
    if (it==ctx.end())
        throw std::runtime_error("Undefined variable: " + expr.name);

    return it->second;
}

double Evaluator::evalUnary(const ast::UnaryExpr& expr,const Context& ctx) {
    double value=evaluate(*expr.operand,ctx);

    switch (expr.op) {
        case ast::UnaryOp::Plus:
            return value;
        case ast::UnaryOp::Minus:
            return -value;
        case ast::UnaryOp::Abs:
            return std::abs(value);
    }

    throw std::runtime_error("Unary operator not recognized");
}

double Evaluator::evalBinary(const ast::BinaryExpr& expr,const Context& ctx) {
    double left=evaluate(*expr.left, ctx);
    double right=evaluate(*expr.right, ctx);

    switch (expr.op) {
        case ast::BinaryOp::Add:
            return left+right;
        case ast::BinaryOp::Sub:
            return left-right;
        case ast::BinaryOp::Mul:
            return left*right;
        case ast::BinaryOp::Div:
            if (right==0.0)
                throw std::runtime_error("Division by zero");
            return left / right;
        case ast::BinaryOp::Pow:
            return std::pow(left, right);
    }

    throw std::runtime_error("Binary operator not recognized");
}