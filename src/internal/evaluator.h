#pragma once

#include <stdexcept>
#include <string>
#include <functional>
#include <unordered_map>

#include "ast.h"

class Evaluator {
   public:
    using Context = std::unordered_map<std::string, double>;

    static double evaluate(const ast::Expr& expr, const Context& ctx);

   private:
    static double evalNumber(const ast::NumberExpr& expr);
    static double evalVariable(const ast::VariableExpr& expr,
                               const Context& ctx);
    static double evalUnary(const ast::UnaryExpr& expr, const Context& ctx);
    static double evalBinary(const ast::BinaryExpr& expr, const Context& ctx);
    static double evalCall(const ast::CallExpr& expr, const Context& ctx);
};