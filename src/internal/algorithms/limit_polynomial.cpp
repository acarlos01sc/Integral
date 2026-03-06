#include "internal/algorithms/limit_polynomial.h"

#include <cmath>

#include "internal/ast.h"

namespace numathap {
namespace internal {

// ---------------------------------------------------------------------------
// detect_degree
// ---------------------------------------------------------------------------
int detect_degree(const ast::Expr* node, const std::string& var) {
    using namespace ast;

    if (!node) return INVALID_DEGREE;

    // --------------------------------------------------------
    // Number
    // --------------------------------------------------------
    if (dynamic_cast<const NumberExpr*>(node)) return 0;

    // --------------------------------------------------------
    // Variable
    // --------------------------------------------------------
    if (auto v = dynamic_cast<const VariableExpr*>(node)) {
        return (v->name == var) ? 1 : -1;
    }

    // --------------------------------------------------------
    // Unary
    // --------------------------------------------------------
    if (auto u = dynamic_cast<const UnaryExpr*>(node)) {
        int d = detect_degree(u->operand.get(), var);
        if (d == INVALID_DEGREE) return INVALID_DEGREE;

        if (u->op == UnaryOp::Plus || u->op == UnaryOp::Minus) return d;

        if (u->op == UnaryOp::Abs) return INVALID_DEGREE;
    }

    // --------------------------------------------------------
    // Binary
    // --------------------------------------------------------
    if (auto b = dynamic_cast<const BinaryExpr*>(node)) {
        int d1 = detect_degree(b->left.get(), var);
        int d2 = detect_degree(b->right.get(), var);

        if (d1 == INVALID_DEGREE || d2 == INVALID_DEGREE) return INVALID_DEGREE;

        switch (b->op) {
            case BinaryOp::Add:
            case BinaryOp::Sub:
                return std::max(d1, d2);

            case BinaryOp::Mul:
                return d1 + d2;

            case BinaryOp::Div:
                return d1 - d2;

            case BinaryOp::Pow:
                if (auto rightNum =
                        dynamic_cast<const NumberExpr*>(b->right.get())) {
                    double exp = rightNum->value;

                    if (std::floor(exp) == exp) {
                        return d1 * static_cast<int>(exp);
                    }
                }
                return INVALID_DEGREE;
        }
    }

    // --------------------------------------------------------
    // Function call → not polynomial
    // --------------------------------------------------------
    if (dynamic_cast<const CallExpr*>(node)) return INVALID_DEGREE;

    return INVALID_DEGREE;
}

LeadingTerm invalid_term() {
    return {0.0, 0, false};
}

// ---------------------------------------------------------------------------
// extract_leading_term
// ---------------------------------------------------------------------------
LeadingTerm extract_leading_term(const ast::Expr* node,
                                        const std::string& var) {
    using namespace ast;

    if (!node) return invalid_term();

    if (auto n = dynamic_cast<const NumberExpr*>(node))
        return {n->value, 0, true};

    if (auto v = dynamic_cast<const VariableExpr*>(node))
        return (v->name == var) ? LeadingTerm{1.0, 1, true}
                                : LeadingTerm{1.0, 0, true};

    if (auto u = dynamic_cast<const UnaryExpr*>(node)) {
        auto t = extract_leading_term(u->operand.get(), var);
        if (!t.valid) return invalid_term();

        if (u->op == UnaryOp::Minus) t.coefficient = -t.coefficient;

        return t;
    }

    if (auto b = dynamic_cast<const BinaryExpr*>(node)) {
        auto L = extract_leading_term(b->left.get(), var);
        auto R = extract_leading_term(b->right.get(), var);

        if (!L.valid || !R.valid) return invalid_term();

        switch (b->op) {
            case BinaryOp::Add:
                return (L.degree >= R.degree) ? L : R;

            case BinaryOp::Sub:
                return (L.degree >= R.degree)
                           ? L
                           : LeadingTerm{-R.coefficient, R.degree, true};

            case BinaryOp::Mul:
                return {L.coefficient * R.coefficient, L.degree + R.degree,
                        true};

            case BinaryOp::Div:
                return {L.coefficient / R.coefficient, L.degree - R.degree,
                        true};

            case BinaryOp::Pow:
                if (auto n = dynamic_cast<const NumberExpr*>(b->right.get())) {
                    double exp = n->value;
                    if (std::floor(exp) == exp) {
                        int e = static_cast<int>(exp);
                        return {std::pow(L.coefficient, e), L.degree * e, true};
                    }
                }
                return invalid_term();
        }
    }

    return invalid_term();
}

}  // namespace internal
}  // namespace numathap