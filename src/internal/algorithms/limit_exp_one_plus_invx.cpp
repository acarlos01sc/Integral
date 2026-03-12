#include "internal/algorithms/limit_exp_one_plus_invx.h"

#include <cmath>

namespace numathap::internal {

namespace {

// ------------------------------------------------------------
// Basic helpers
// ------------------------------------------------------------

bool is_variable(const ast::Expr* e, const std::string& var) {
    auto v = dynamic_cast<const ast::VariableExpr*>(e);
    return v && v->name == var;
}

bool is_number(const ast::Expr* e, double& value) {
    auto n = dynamic_cast<const ast::NumberExpr*>(e);
    if (!n) return false;
    value = n->value;
    return true;
}

// ------------------------------------------------------------
// Match k*x
// ------------------------------------------------------------

bool match_kx(const ast::Expr* e, const std::string& var, double& k) {
    if (is_variable(e, var)) {
        k = 1.0;
        return true;
    }

    auto mul = dynamic_cast<const ast::BinaryExpr*>(e);
    if (!mul || mul->op != ast::BinaryOp::Mul) return false;

    double c;

    if (is_number(mul->left.get(), c) && is_variable(mul->right.get(), var)) {
        k = c;
        return true;
    }

    if (is_number(mul->right.get(), c) && is_variable(mul->left.get(), var)) {
        k = c;
        return true;
    }

    return false;
}

// ------------------------------------------------------------
// Match k*x + a
// ------------------------------------------------------------

bool match_kx_plus_a(const ast::Expr* e, const std::string& var, double& k,
                     double& a) {
    auto add = dynamic_cast<const ast::BinaryExpr*>(e);
    if (!add) return false;

    double c;

    if (add->op == ast::BinaryOp::Add) {
        if (match_kx(add->left.get(), var, k) &&
            is_number(add->right.get(), c)) {
            a = c;
            return true;
        }

        if (match_kx(add->right.get(), var, k) &&
            is_number(add->left.get(), c)) {
            a = c;
            return true;
        }
    }

    if (add->op == ast::BinaryOp::Sub) {
        if (match_kx(add->left.get(), var, k) &&
            is_number(add->right.get(), c)) {
            a = -c;
            return true;
        }
    }

    return false;
}

// ------------------------------------------------------------
// Match b*x
// ------------------------------------------------------------

bool match_bx(const ast::Expr* e, const std::string& var, double& b) {
    if (is_variable(e, var)) {
        b = 1.0;
        return true;
    }

    auto mul = dynamic_cast<const ast::BinaryExpr*>(e);
    if (!mul || mul->op != ast::BinaryOp::Mul) return false;

    double c;

    if (is_number(mul->left.get(), c) && is_variable(mul->right.get(), var)) {
        b = c;
        return true;
    }

    if (is_number(mul->right.get(), c) && is_variable(mul->left.get(), var)) {
        b = c;
        return true;
    }

    return false;
}

// ------------------------------------------------------------
// Match 1 + a/x
// ------------------------------------------------------------

bool match_one_plus_a_over_x(const ast::Expr* e, const std::string& var,
                             double& a) {
    auto add = dynamic_cast<const ast::BinaryExpr*>(e);
    if (!add || add->op != ast::BinaryOp::Add) return false;

    double c;

    // 1 + a/x
    if (is_number(add->left.get(), c) && c == 1.0) {
        auto div = dynamic_cast<const ast::BinaryExpr*>(add->right.get());
        if (!div || div->op != ast::BinaryOp::Div) return false;

        double num;

        if (is_number(div->left.get(), num) &&
            is_variable(div->right.get(), var)) {
            a = num;
            return true;
        }
    }

    // a/x + 1
    if (is_number(add->right.get(), c) && c == 1.0) {
        auto div = dynamic_cast<const ast::BinaryExpr*>(add->left.get());
        if (!div || div->op != ast::BinaryOp::Div) return false;

        double num;

        if (is_number(div->left.get(), num) &&
            is_variable(div->right.get(), var)) {
            a = num;
            return true;
        }
    }

    return false;
}

}  // namespace

// ------------------------------------------------------------
// Main heuristic
// ------------------------------------------------------------

std::optional<double> limit_exp_one_plus_invx(const ast::Expr* expr,
                                              const std::string& var,
                                              bool infinite,
                                              bool negative_inf) {
    (void)negative_inf;
    
    if (!infinite) return std::nullopt;

    auto pow = dynamic_cast<const ast::BinaryExpr*>(expr);
    if (!pow || pow->op != ast::BinaryOp::Pow) return std::nullopt;

    const ast::Expr* base = pow->left.get();
    const ast::Expr* exponent = pow->right.get();

    double b;

    if (!match_bx(exponent, var, b)) return std::nullopt;

    double a;
    double k1, k2;

    bool matched = false;

    // --------------------------------------------------------
    // Case ((k*x+a)/(k*x))
    // --------------------------------------------------------

    if (auto div = dynamic_cast<const ast::BinaryExpr*>(base)) {
        if (div->op == ast::BinaryOp::Div &&
            match_kx_plus_a(div->left.get(), var, k1, a) &&
            match_kx(div->right.get(), var, k2) && std::abs(k1 - k2) < 1e-12) {
            a = a / k1;
            matched = true;
        }
    }

    // --------------------------------------------------------
    // Case (1 + a/x)
    // --------------------------------------------------------

    if (!matched) {
        if (match_one_plus_a_over_x(base, var, a)) {
            matched = true;
        }
    }

    if (!matched) return std::nullopt;

    double result = std::exp(a * b);

    return result;
}

}  // namespace numathap::internal