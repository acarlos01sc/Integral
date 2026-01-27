#include "evaluator.h"

#include <cmath>
#include <functional>
#include <stdexcept>
#include <unordered_map>

#include "ast.h"

double Evaluator::evaluate(const ast::Expr& expr, const Context& ctx) {
    if (auto num = dynamic_cast<const ast::NumberExpr*>(&expr))
        return evalNumber(*num);
    if (auto var = dynamic_cast<const ast::VariableExpr*>(&expr))
        return evalVariable(*var, ctx);
    if (auto un = dynamic_cast<const ast::UnaryExpr*>(&expr))
        return evalUnary(*un, ctx);
    if (auto bin = dynamic_cast<const ast::BinaryExpr*>(&expr))
        return evalBinary(*bin, ctx);
    if (auto call = dynamic_cast<const ast::CallExpr*>(&expr))
        return evalCall(*call, ctx);

    throw std::runtime_error("Undefined expression type");
}

double Evaluator::evalNumber(const ast::NumberExpr& expr) { return expr.value; }

double Evaluator::evalVariable(const ast::VariableExpr& expr,
                               const Context& ctx) {
    static const std::unordered_map<std::string, double> constants{
        {"pi", M_PI},
        {"e", M_E},
    };
    if (auto it = constants.find(expr.name); it != constants.end())
        return it->second;
    if (auto it = ctx.find(expr.name); it != ctx.end()) return it->second;

    throw std::runtime_error("Undefined variable: " + expr.name);
}

double Evaluator::evalUnary(const ast::UnaryExpr& expr, const Context& ctx) {
    double value = evaluate(*expr.operand, ctx);

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

double Evaluator::evalBinary(const ast::BinaryExpr& expr, const Context& ctx) {
    double left = evaluate(*expr.left, ctx);
    double right = evaluate(*expr.right, ctx);

    switch (expr.op) {
        case ast::BinaryOp::Add:
            return left + right;
        case ast::BinaryOp::Sub:
            return left - right;
        case ast::BinaryOp::Mul:
            return left * right;
        case ast::BinaryOp::Div:
            if (right == 0.0) throw std::runtime_error("Division by zero");
            return left / right;
        case ast::BinaryOp::Pow:
            return std::pow(left, right);
    }

    throw std::runtime_error("Binary operator not recognized");
}

double Evaluator::evalCall(const ast::CallExpr& expr, const Context& ctx) {
    static const std::unordered_map<
                                    std::string,
                                    double (*)(double)
                                    > functions{
                                                {"sin", std::sin},
                                                {"cos", std::cos},
                                                {"tan", std::tan},
                                                {"log", std::log},
                                                {"exp", std::exp},
                                                {"sqrt", std::sqrt},
                                                {"abs", std::abs},
        };
    auto it = functions.find(expr.callee);
    if (it == functions.end())
        throw std::runtime_error("Unknow function " + expr.callee);

    double arg = evaluate(*expr.argument, ctx);
    return it->second(arg);
}