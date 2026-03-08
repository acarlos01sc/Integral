#include <numathap/limit.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "internal/algorithms/limit_forward.h"
#include "internal/algorithms/limit_polynomial.h"
#include "internal/algorithms/limit_richardson.h"
#include "internal/algorithms/limit_sqrt_minus_const.h"
#include "internal/ast.h"
#include "internal/evaluator.h"
#include "internal/lexer.h"
#include "internal/numeric_utils.h"
#include "internal/parser.h"

namespace numathap {

namespace {

// ------------------------------------------------------------
// Check if a double represents infinity
// ------------------------------------------------------------
bool is_infinite(double x) { return std::isinf(x); }

// ---------------------------------------------------------------------------
// dispatch_limit
// ---------------------------------------------------------------------------
// Executes the numerical limit method based on options and type of limit.
// For LimitMethod::Auto:
//   - Infinite limits use Forward sampling
//   - Finite limits attempt Richardson first, fallback to Forward
// ---------------------------------------------------------------------------
template <typename Func>
LimitResult dispatch_limit(Func&& f, double point, const LimitOptions& options,
                           bool infinite, bool negative_infinite) {
    LimitMethod method = options.method;

    if (method == LimitMethod::Auto) {
        if (infinite) {
            method = LimitMethod::Forward;
        } else {
            try {
                auto r = internal::limit_richardson(f, point, options);
                if (r.status == LimitStatus::Converged) return r;
            } catch (...) { /* ignore */ }

            return internal::limit_forward(f, point, options, false, false);
        }
    }

    switch (method) {
        case LimitMethod::Forward:
            return internal::limit_forward(f, point, options, infinite,
                                           negative_infinite);
        case LimitMethod::Richardson:
            if (infinite)
                throw std::runtime_error("Richardson not supported for infinity");
            return internal::limit_richardson(f, point, options);
        default:
            throw std::runtime_error("Limit method not implemented");
    }
}

} // anonymous namespace

// ============================================================================
// limit()
// ----------------------------------------------------------------------------
// Computes the limit of an expression at a point (finite or infinite).
// Heuristics for polynomials and rational expressions are attempted first.
// If they are inconclusive, numerical methods (Forward/Richardson) are used.
// ============================================================================
LimitResult limit(const std::string& expression, const std::string& variable,
                  const std::string& value, const LimitOptions& options) {
    if (options.abs_tolerance <= 0.0)
        throw std::invalid_argument("abs_tolerance must be positive");
    if (options.rel_tolerance <= 0.0)
        throw std::invalid_argument("rel_tolerance must be positive");
    if (options.max_iterations <= 0)
        throw std::invalid_argument("max_iterations must be positive");

    // ------------------------------------------------------------
    // Parse expression
    // ------------------------------------------------------------
    Lexer exprLexer(expression);
    Parser exprParser(exprLexer);
    auto expr = exprParser.parse();

    // ------------------------------------------------------------
    // Apply sqrt(expr) - constant heuristic if available
    // ------------------------------------------------------------
    if (auto rewritten = internal::transform_sqrt_minus_const(expr.get())) {
        return limit(*rewritten, variable, value, options);
    }

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

    // ------------------------------------------------------------
    // Build numerical function f(x) evaluating the AST
    // ------------------------------------------------------------
    Evaluator::Context ctx;
    auto f = [&](double x) -> double {
        ctx[variable] = x;
        double val = Evaluator::evaluate(*expr, ctx);
        if (std::isnan(val)) return std::numeric_limits<double>::quiet_NaN();
        return val;
    };

    // ---------------------------------------------------------------------------
    // Heuristic: limits at infinity (polynomials)
    // ---------------------------------------------------------------------------
    if (infinite) {
        if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr.get())) {
            if (b->op == ast::BinaryOp::Div) {
                auto num = internal::extract_leading_term(b->left.get(), variable);
                auto den = internal::extract_leading_term(b->right.get(), variable);
                if (num.valid && den.valid) {
                    if (num.degree < den.degree) return {0.0, LimitStatus::Converged, 0};
                    if (num.degree == den.degree)
                        return {num.coefficient / den.coefficient,
                                LimitStatus::Converged, 0};
                    // For higher degree, cannot be sure; fallback to numerical
                }
            }
        }

        int degree = internal::detect_degree(expr.get(), variable);
        if (degree != internal::INVALID_DEGREE) {
            if (degree < 0) return {0.0, LimitStatus::Converged, 0};
            if (degree == 0) {
                Evaluator::Context tmp;
                double scale = std::sqrt(std::numeric_limits<double>::max()) * 1e-3;
                tmp[variable] = negative_infinite ? -scale : scale;
                double val = Evaluator::evaluate(*expr, tmp);
                return {internal::finalize_value(val, options.abs_tolerance),
                        LimitStatus::Converged, 0};
            }
            // degree > 0: cannot be sure, fallback
        }
       //return dispatch_limit(f, point, options, true, negative_infinite);
    }

    // ---------------------------------------------------------------------------
    // Heuristic: rational expression near zero
    // ---------------------------------------------------------------------------
    if (!infinite && std::abs(point) < options.abs_tolerance) {
        if (auto b = dynamic_cast<const ast::BinaryExpr*>(expr.get())) {
            if (b->op == ast::BinaryOp::Div) {
                auto num = internal::extract_lowest_term(b->left.get(), variable);
                auto den = internal::extract_lowest_term(b->right.get(), variable);

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

                    if (std::isfinite(result))
                        return {internal::finalize_value(result, options.abs_tolerance),
                                LimitStatus::Converged, 0};
                    // Otherwise fallback to numerical
                }
            }
        }
    }

    // ---------------------------------------------------------------------------
    // Bilateral limit: compute left and right independently
    // ---------------------------------------------------------------------------
    if (options.side == LimitSide::Both && !infinite) {
        LimitOptions left_options = options;
        left_options.side = LimitSide::Left;
        LimitOptions right_options = options;
        right_options.side = LimitSide::Right;

        LimitResult left = dispatch_limit(f, point, left_options, false, false);
        LimitResult right = dispatch_limit(f, point, right_options, false, false);

        if (left.status == LimitStatus::Converged &&
            right.status == LimitStatus::Converged) {
            double diff = std::abs(left.value - right.value);
            double tol = std::max(options.abs_tolerance,
                                  options.rel_tolerance * std::abs(right.value));
            if (diff < tol) {
                return {internal::finalize_value(right.value, options.abs_tolerance),
                        LimitStatus::Converged,
                        std::max(left.iterations, right.iterations)};
            }
            return {std::numeric_limits<double>::quiet_NaN(),
                    LimitStatus::Undefined,
                    std::max(left.iterations, right.iterations)};
        }
        if (left.status == LimitStatus::Divergent &&
            right.status == LimitStatus::Divergent &&
            std::isinf(left.value) && std::isinf(right.value) &&
            (std::signbit(left.value) == std::signbit(right.value))) {
            return {left.value, LimitStatus::Divergent,
                    std::max(left.iterations, right.iterations)};
        }
        return {std::numeric_limits<double>::quiet_NaN(),
                LimitStatus::Undefined,
                std::max(left.iterations, right.iterations)};
    }

    // ---------------------------------------------------------------------------
    // Default: single-sided or infinite, fallback to numerical
    // ---------------------------------------------------------------------------
    return dispatch_limit(f, point, options, infinite, negative_infinite);
}

} // namespace numathap