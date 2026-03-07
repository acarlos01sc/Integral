#include "internal/algorithms/limit_sqrt_minus_const.h"

#include <cmath>
#include <optional>
#include <sstream>

#include "internal/ast_to_string.h"

namespace numathap::internal {

static bool is_constant(const ast::Expr* e, double& value) {
    if (auto n = dynamic_cast<const ast::NumberExpr*>(e)) {
        value = n->value;
        return true;
    }
    return false;
}

std::optional<std::string> transform_sqrt_minus_const(const ast::Expr* expr) {
    const auto* bin = dynamic_cast<const ast::BinaryExpr*>(expr);

    if (!bin) return std::nullopt;

    if (bin->op == ast::BinaryOp::Div) {
        auto left = transform_sqrt_minus_const(bin->left.get());
        auto right = transform_sqrt_minus_const(bin->right.get());

        if (left && right) {
            std::ostringstream out;

            out << "(" << *left << ")/(" << *right << ")";

            return out.str();
        }

        return std::nullopt;
    }

    if (bin->op != ast::BinaryOp::Sub) return std::nullopt;

    const auto* call = dynamic_cast<const ast::CallExpr*>(bin->left.get());

    if (!call) return std::nullopt;

    if (call->callee != "sqrt") return std::nullopt;

    double constant;

    if (!is_constant(bin->right.get(), constant)) return std::nullopt;

    const ast::Expr* inner = call->argument.get();

    std::string inner_str = ast_to_string(inner);

    std::ostringstream out;

    out << "((" << inner_str << ") - " << (constant * constant) << ")";
    out << "/(sqrt(" << inner_str << ") + " << constant << ")";

    return out.str();
}

}  // namespace numathap::internal