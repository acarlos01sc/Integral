#include <numathap/limit.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "internal/algorithms/limit_forward.h"
#include "internal/algorithms/limit_richardson.h"
#include "internal/ast.h"
#include "internal/evaluator.h"
#include "internal/lexer.h"
#include "internal/numeric_utils.h"
#include "internal/parser.h"

namespace numathap {

namespace {

// ------------------------------------------------------------
// Parse limit value (finite or infinite)
// ------------------------------------------------------------
bool is_infinite(double x) { return !std::isfinite(x); }

constexpr int INVALID_DEGREE = std::numeric_limits<int>::min();

// ---------------------------------------------------------------------------
// detect_degree
// ---------------------------------------------------------------------------
// Attempts to determine the polynomial degree of an expression with respect
// to a given variable using structural analysis of the AST.
//
// The function supports a restricted class of expressions:
//
//   - constants
//   - variables
//   - unary +/-
//   - +, -, *, /
//   - powers with integer exponent
//
// If the expression cannot be safely interpreted as a polynomial structure,
// the function returns INVALID_DEGREE.
//
// This heuristic is used to analytically resolve limits at infinity when
// possible.
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

        // +x ou -x não altera grau
        if (u->op == UnaryOp::Plus || u->op == UnaryOp::Minus) return d;

        // |x| → não tratamos estruturalmente
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
                // só tratamos x^n com n inteiro
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
    // CallExpr (sin, cos, exp, etc)
    // Not Polinomial
    // --------------------------------------------------------
    if (dynamic_cast<const CallExpr*>(node)) return INVALID_DEGREE;

    return INVALID_DEGREE;
}

// ---------------------------------------------------------------------------
// dispatch_limit
// ---------------------------------------------------------------------------
// Selects and executes the appropriate numerical limit algorithm.
//
// If LimitMethod::Auto is selected:
//
//   - Infinite limits use the Forward method
//   - Finite limits attempt Richardson extrapolation first
//   - If Richardson fails, the algorithm falls back to Forward sampling
//
// This strategy improves robustness for difficult functions.
// ---------------------------------------------------------------------------

template <typename Func>
LimitResult dispatch_limit(Func&& f, double point, const LimitOptions& options,
                           bool infinite, bool negative_infinite) {
    LimitMethod method = options.method;

    if (method == LimitMethod::Auto) {
        if (infinite) {
            method = LimitMethod::Forward;
        } else {
            // Richardson
            try {
                auto r = internal::limit_richardson(f, point, options);

                if (r.status == LimitStatus::Converged) {
                    return r;
                }

            } catch (...) {
                // ignore
            }

            // Fallback
            return internal::limit_forward(f, point, options, false, false);
        }
    }

    switch (method) {
        case LimitMethod::Forward:
            return internal::limit_forward(f, point, options, infinite,
                                           negative_infinite);

        case LimitMethod::Richardson:
            if (infinite) {
                throw std::runtime_error(
                    "Richardson method not supported for infinite limits");
            }
            return internal::limit_richardson(f, point, options);

        default:
            throw std::runtime_error("Limit method not implemented");
    }
}

}  // anonymous namespace

// ============================================================================
// limit()
// ----------------------------------------------------------------------------
// Computes the limit of a mathematical expression.
//
// The function follows a hybrid strategy:
//
// 1. The expression is parsed into an AST.
// 2. For limits at infinity, a structural analysis attempts to detect the
//    polynomial degree of the expression.
// 3. If structural analysis succeeds, the limit may be resolved analytically.
// 4. Otherwise, a numerical limit algorithm is applied.
//
// Numerical methods currently supported:
//
//   - Forward sampling
//   - Richardson extrapolation
//
// Bilateral limits are computed by evaluating the left and right limits
// independently and comparing them within the requested tolerances.
//
// Parameters:
//   expression  : mathematical expression
//   variable    : variable with respect to which the limit is taken
//   value       : limit point (finite or infinite)
//   options     : numerical options controlling the algorithm
//
// Returns:
//   LimitResult structure containing value, status, and iteration count.
// ============================================================================

LimitResult limit(const std::string& expression, const std::string& variable,
                  const std::string& value, const LimitOptions& options) {
    if (options.abs_tolerance <= 0.0) {
        throw std::invalid_argument("abs_tolerance must be positive");
    }

    if (options.rel_tolerance <= 0.0) {
        throw std::invalid_argument("rel_tolerance must be positive");
    }

    if (options.max_iterations <= 0) {
        throw std::invalid_argument("max_iterations must be positive");
    }

    // ------------------------------------------------------------
    // Parse expression
    // ------------------------------------------------------------
    Lexer exprLexer(expression);
    Parser exprParser(exprLexer);
    auto expr = exprParser.parse();

    // ------------------------------------------------------------
    // Parse limit value
    // ------------------------------------------------------------
    Lexer valueLexer(value);
    Parser valueParser(valueLexer);
    auto valueExpr = valueParser.parse();

    Evaluator::Context empty;

    double point = Evaluator::evaluate(*valueExpr, empty);

    bool infinite = is_infinite(point);
    bool negative_infinite = infinite && std::signbit(point);

    // Build numerical function f(x) that evaluates the parsed AST.
    // The evaluator context is reused across calls to avoid allocations.

    Evaluator::Context ctx;

    auto f = [&](double x) -> double {
        ctx[variable] = x;
        double val = Evaluator::evaluate(*expr, ctx);

        if (std::isnan(val)) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        if (std::isinf(val)) {
            return val;
        }

        return val;
    };

    // ---------------------------------------------------------------------------
    // Structural rational limit at finite point
    //
    // Detects limits of rational expressions using leading-term analysis.
    // This resolves indeterminate forms such as:
    //
    //   (4*x^3 - 2*x^2 + x) / (3*x^2 + 2*x)  as x -> 0
    //
    // without relying on numerical sampling.
    // ---------------------------------------------------------------------------

    /*if (!infinite && std::abs(point) > options.abs_tolerance) {
        if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr.get())) {
            if (b->op == ast::BinaryOp::Div) {
                auto num =
                    internal::extract_leading_term(b->left.get(), variable);
                auto den =
                    internal::extract_leading_term(b->right.get(), variable);

                if (num.valid && den.valid && den.coefficient != 0.0) {
                    double result;

                    if (num.degree > den.degree) {
                        result = 0.0;
                    } else if (num.degree < den.degree) {
                        result = (num.coefficient / den.coefficient > 0)
                                     ? std::numeric_limits<double>::infinity()
                                     : -std::numeric_limits<double>::infinity();
                    } else {
                        result = num.coefficient / den.coefficient;
                    }

                    return {
                        internal::finalize_value(result, options.abs_tolerance),
                        std::isfinite(result) ? LimitStatus::Converged
                                              : LimitStatus::Divergent,
                        0};
                }
            }
        }
    }
*/
    // ---------------------------------------------------------------------------
    // Structural handling for limits at infinity
    //
    // If the expression behaves like a polynomial, we can determine the limit
    // analytically using its detected degree:
    //
    //   degree < 0  →  limit = 0
    //   degree = 0  →  constant asymptotic value
    //   degree > 0  →  divergent limit
    //
    // If degree detection fails, the algorithm falls back to numerical methods.
    // ---------------------------------------------------------------------------

    if (infinite) {
        if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr.get())) {
            if (b->op == ast::BinaryOp::Div) {
                auto num =
                    internal::extract_leading_term(b->left.get(), variable);
                auto den =
                    internal::extract_leading_term(b->right.get(), variable);

                if (num.valid && den.valid) {
                    if (num.degree < den.degree)
                        return {0.0, LimitStatus::Converged, 0};

                    if (num.degree == den.degree)
                        return {num.coefficient / den.coefficient,
                                LimitStatus::Converged, 0};

                    double sign =
                        (num.coefficient / den.coefficient) > 0 ? 1.0 : -1.0;

                    if (negative_infinite && ((num.degree - den.degree) % 2))
                        sign = -sign;

                    return {sign * std::numeric_limits<double>::infinity(),
                            LimitStatus::Divergent, 0};
                }
            }
        }

        int degree = detect_degree(expr.get(), variable);

        if (degree != INVALID_DEGREE) {
            // grau < 0 → 0
            if (degree < 0) return {0.0, LimitStatus::Converged, 0};

            // grau == 0 → constante
            if (degree == 0) {
                Evaluator::Context tmp;
                double scale =
                    std::sqrt(std::numeric_limits<double>::max()) * 1e-3;

                tmp[variable] = negative_infinite ? -scale : scale;

                double val = Evaluator::evaluate(*expr, tmp);

                return {internal::finalize_value(val, options.abs_tolerance),
                        LimitStatus::Converged, 0};
            }

            // grau > 0 → divergente
            double sign = 1.0;

            if (negative_infinite && (degree % 2 != 0)) sign = -1.0;

            return {sign * std::numeric_limits<double>::infinity(),
                    LimitStatus::Divergent, 0};
        }

        // fallback numérico
        return dispatch_limit(f, point, options, true, negative_infinite);
    }

    // ---------------------------------------------------------------------------
    // Structural rational limit near zero
    //
    // Detects limits of rational expressions by comparing the lowest-degree
    // terms of numerator and denominator.
    //
    // Example:
    //
    // (4*x^3-2*x^2+x)/(3*x^2+2*x)  as x -> 0  →  1/2
    // ---------------------------------------------------------------------------

    if (!infinite && std::abs(point) < options.abs_tolerance) {
        if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr.get())) {
            if (b->op == ast::BinaryOp::Div) {
                auto num =
                    internal::extract_lowest_term(b->left.get(), variable);
                auto den =
                    internal::extract_lowest_term(b->right.get(), variable);

                if (num.valid && den.valid && den.coefficient != 0.0) {
                    double result;

                    if (num.degree > den.degree)
                        result = 0.0;
                    else if (num.degree < den.degree)
                        result = (num.coefficient / den.coefficient > 0)
                                     ? std::numeric_limits<double>::infinity()
                                     : -std::numeric_limits<double>::infinity();
                    else
                        result = num.coefficient / den.coefficient;

                    return {
                        internal::finalize_value(result, options.abs_tolerance),
                        std::isfinite(result) ? LimitStatus::Converged
                                              : LimitStatus::Divergent,
                        0};
                }
            }
        }
    }

    // ---------------------------------------------------------------------------
    // Bilateral limit computation
    //
    // The left and right limits are computed independently and compared using
    // absolute and relative tolerances. If both sides converge to the same
    // value within tolerance, the bilateral limit is accepted.
    //
    // If both sides diverge to infinities with the same sign, the result is
    // considered a divergent limit.
    // ---------------------------------------------------------------------------

    if (options.side == LimitSide::Both && !infinite) {
        LimitOptions left_options = options;
        left_options.side = LimitSide::Left;

        LimitOptions right_options = options;
        right_options.side = LimitSide::Right;

        LimitResult left = dispatch_limit(f, point, left_options, false, false);

        LimitResult right =
            dispatch_limit(f, point, right_options, false, false);

        if (left.status == LimitStatus::Converged &&
            right.status == LimitStatus::Converged) {
            double diff = std::abs(left.value - right.value);
            double tol =
                std::max(options.abs_tolerance,
                         options.rel_tolerance * std::abs(right.value));

            if (diff < tol) {
                return {internal::finalize_value(right.value,
                                                 options.abs_tolerance),
                        LimitStatus::Converged,
                        std::max(left.iterations, right.iterations)};
            }

            return {std::numeric_limits<double>::quiet_NaN(),
                    LimitStatus::Undefined,
                    std::max(left.iterations, right.iterations)};
        }

        // --------------------------------------------------------
        // Bilateral infinite divergence handling
        // --------------------------------------------------------
        if (left.status == LimitStatus::Divergent &&
            right.status == LimitStatus::Divergent) {
            if (std::isinf(left.value) && std::isinf(right.value) &&
                (std::signbit(left.value) == std::signbit(right.value))) {
                return {left.value, LimitStatus::Divergent,
                        std::max(left.iterations, right.iterations)};
            }
        }

        return {std::numeric_limits<double>::quiet_NaN(),
                LimitStatus::Undefined,
                std::max(left.iterations, right.iterations)};
    }

    // ------------------------------------------------------------
    // Single-sided or infinite case
    // ------------------------------------------------------------
    return dispatch_limit(f, point, options, infinite, negative_infinite);
}

}  // namespace numathap