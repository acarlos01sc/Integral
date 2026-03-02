#include <numathap/limit.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "internal/algorithms/limit_forward.h"
#include "internal/algorithms/limit_richardson.h"
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

// ------------------------------------------------------------
// Dispatch limit method
// ------------------------------------------------------------
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

// ----------------------------------------------------------------------
// Public limit()
// ----------------------------------------------------------------------

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

    // ------------------------------------------------------------
    // Build numerical function f(x)
    // ------------------------------------------------------------
    Evaluator::Context ctx;

    auto f = [&](double x) -> double {
        ctx[variable] = x;
        double val = Evaluator::evaluate(*expr, ctx);

        if (!std::isfinite(val)) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        return val;
    };

    // ------------------------------------------------------------
    // Bilateral case
    // ------------------------------------------------------------
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