#include "limit_bounded_product.h"

#include <cmath>
#include "internal/ast.h"

namespace numathap::internal {

namespace {

// ------------------------------------------------------------
// Detect if expression is bounded
// ------------------------------------------------------------
bool is_bounded_function(const ast::Expr* expr) {

    auto call = dynamic_cast<const ast::CallExpr*>(expr);

    if (!call)
        return false;

    const std::string& name = call->callee;

    if (name == "sin") return true;
    if (name == "cos") return true;
    if (name == "tanh") return true;

    return false;

}

// ------------------------------------------------------------
// Detect x^p
// ------------------------------------------------------------
bool extract_power_of_variable(
    const ast::Expr* expr,
    const std::string& variable,
    double& exponent)
{
    if (auto v = dynamic_cast<const ast::VariableExpr*>(expr)) {
        if (v->name == variable) {
            exponent = 1.0;
            return true;
        }
    }

    if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr)) {

        if (b->op == ast::BinaryOp::Pow) {

            auto base = dynamic_cast<const ast::VariableExpr*>(b->left.get());
            auto exp  = dynamic_cast<const ast::NumberExpr*>(b->right.get());

            if (base && exp && base->name == variable) {
                exponent = exp->value;
                return true;
            }
        }
    }

    return false;
}

}

// ------------------------------------------------------------
// try_bounded_product
// ------------------------------------------------------------
std::optional<LimitResult> try_bounded_product(
    const ast::Expr* expr,
    const std::string& variable,
    double point,
    const LimitOptions&)
{
    if (point != 0.0) return std::nullopt;

    auto mul = dynamic_cast<const ast::BinaryExpr*>(expr);

    if (!mul || mul->op != ast::BinaryOp::Mul)
        return std::nullopt;

    double power;

    if (extract_power_of_variable(mul->left.get(), variable, power) &&
        is_bounded_function(mul->right.get()))
    {
        if (power > 0)
            return LimitResult{0.0, LimitStatus::Converged, 0};
    }

    if (extract_power_of_variable(mul->right.get(), variable, power) &&
        is_bounded_function(mul->left.get()))
    {
        if (power > 0)
            return LimitResult{0.0, LimitStatus::Converged, 0};
    }

    return std::nullopt;
}

}