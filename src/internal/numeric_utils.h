#pragma once
#include <cmath>
#include <limits>

#include "internal/ast.h"

#include "internal/algorithms/limit_polynomial.h"

namespace numathap::internal {

inline double finalize_value(double x, double tol) {
    if (!std::isfinite(x)) return x;

    if (std::abs(x) < tol) return 0.0;

    if (std::abs(x) < 10 * std::numeric_limits<double>::epsilon()) return 0.0;

    return x;
}

inline LeadingTerm invalid_term() { return {0.0, 0, false}; }

inline bool try_rational_zero_limit(const ast::Expr* numerator,
                                    const ast::Expr* denominator,
                                    const std::string& var,
                                    double& result) {

    auto num = extract_leading_term(numerator, var);
    auto den = extract_leading_term(denominator, var);

    if (!num.valid || !den.valid)
        return false;

    if (den.coefficient == 0.0)
        return false;

    if (num.degree > den.degree) {
        result = 0.0;
        return true;
    }

    if (num.degree < den.degree) {
        result = (num.coefficient / den.coefficient > 0)
                     ? std::numeric_limits<double>::infinity()
                     : -std::numeric_limits<double>::infinity();
        return true;
    }

    result = num.coefficient / den.coefficient;
    return true;
}

inline LeadingTerm extract_lowest_term(const ast::Expr* node,
                                       const std::string& var) {
    using namespace ast;

    if (!node) return invalid_term();

    if (auto n = dynamic_cast<const NumberExpr*>(node))
        return {n->value, 0, true};

    if (auto v = dynamic_cast<const VariableExpr*>(node))
        return (v->name == var) ? LeadingTerm{1.0, 1, true}
                                : LeadingTerm{1.0, 0, true};

    if (auto u = dynamic_cast<const UnaryExpr*>(node)) {
        auto t = extract_lowest_term(u->operand.get(), var);
        if (!t.valid) return invalid_term();

        if (u->op == UnaryOp::Minus) t.coefficient = -t.coefficient;

        return t;
    }

    if (auto b = dynamic_cast<const BinaryExpr*>(node)) {

        auto L = extract_lowest_term(b->left.get(), var);
        auto R = extract_lowest_term(b->right.get(), var);

        if (!L.valid || !R.valid) return invalid_term();

        switch (b->op) {

            case BinaryOp::Add:
                return (L.degree <= R.degree) ? L : R;

            case BinaryOp::Sub:
                return (L.degree <= R.degree)
                           ? L
                           : LeadingTerm{-R.coefficient, R.degree, true};

            case BinaryOp::Mul:
                return {L.coefficient * R.coefficient,
                        L.degree + R.degree,
                        true};

            case BinaryOp::Div:
                return {L.coefficient / R.coefficient,
                        L.degree - R.degree,
                        true};

            case BinaryOp::Pow:
                if (auto n = dynamic_cast<const NumberExpr*>(b->right.get())) {
                    double exp = n->value;
                    if (std::floor(exp) == exp) {
                        int e = static_cast<int>(exp);
                        return {std::pow(L.coefficient, e),
                                L.degree * e,
                                true};
                    }
                }
                return invalid_term();
        }
    }

    return invalid_term();
}

}  // namespace numathap::internal