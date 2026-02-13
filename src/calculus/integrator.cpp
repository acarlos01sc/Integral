#include <numathap/integrator.h>

#include <cmath>
#include <stdexcept>

#include "internal/algorithms/gauss_kronrod.h"
#include "internal/algorithms/simpson.h"
#include "internal/evaluator.h"
#include "internal/lexer.h"
#include "internal/parser.h"

namespace numathap {

namespace {
// -------------------------------------------------------------------
// Dispatch finite interval integration
// -------------------------------------------------------------------
template <typename Func>
double dispatch_finite(Func&& f, double a, double b,
                       const IntegratorOptions& options) {
    switch (options.method) {
        case IntegrationMethod::AdaptiveSimpson:
            return internal::adaptive_simpson(f, a, b, options.abs_tol,
                                              options.maxDepth);
        case IntegrationMethod::GaussKronrod:
            return internal::gauss_kronrod(f, a, b, options.abs_tol,
                                           options.maxDepth, options.gk_rule);
        default:
            throw std::runtime_error("Integration method not implemented");
    }
}

}  // anonymous namespace

// ----------------------------------------------------------------------
// Public integrate()
// ----------------------------------------------------------------------

double integrate(const std::string& function, const std::string& variable,
                 const std::string& lower, const std::string& upper,
                 const IntegratorOptions& options) {
    if (options.abs_tol <= 0.0) {
        throw std::invalid_argument("abs_tol must be a positive number");
    }

    if (options.maxDepth <= 0) {
        throw std::invalid_argument("maxDepth must be positive");
    }

    // Parse the integrand
    Lexer funcLexer(function);
    Parser funcParser(funcLexer);
    auto funcExpr = funcParser.parse();

    // Parse lower limit
    Lexer lowerLexer(lower);
    Parser lowerParser(lowerLexer);
    auto lowerExpr = lowerParser.parse();

    // Parse upper limit
    Lexer upperLexer(upper);
    Parser upperParser(upperLexer);
    auto upperExpr = upperParser.parse();

    Evaluator::Context empty;

    double a = Evaluator::evaluate(*lowerExpr, empty);
    double b = Evaluator::evaluate(*upperExpr, empty);

    bool lower_inf = !std::isfinite(a);
    bool upper_inf = !std::isfinite(b);

    double sign = 1.0;
    if (a > b) {
        std::swap(a, b);
        sign = -1.0;
    }

    // ----------------------------
    // numerical integrand f(x)
    // ----------------------------
    Evaluator::Context ctx;

    auto f = [&](double x) -> double {
        ctx[variable] = x;
        double val = Evaluator::evaluate(*funcExpr, ctx);
        if (!std::isfinite(val)) {
            throw std::runtime_error("Function evaluation produced NaN or Inf");
        }
        return val;
    };

    // ============================================================
    // Case 1: Proper integral
    // ============================================================
    if (!lower_inf && !upper_inf) {
        return sign * dispatch_finite(f, a, b, options);
    }

    // Small epsilon to avoid exact singular endpoints in (0,1)
    const double eps = 1e-12;

    // ============================================================
    // Case 2: a finite, b = +∞
    // x = a + t/(1-t)
    // ============================================================
    if (!lower_inf && upper_inf) {
        auto g = [&](double t) {
            if (t <= 0.0) return f(a);
            if (t >= 1.0) t = 1.0 - eps;

            double x = a + t / (1.0 - t);
            double jac = 1.0 / ((1.0 - t) * (1.0 - t));
            return f(x) * jac;
        };

        return sign * dispatch_finite(g, 0.0, 1.0 - eps, options);
    }

    // ============================================================
    // Case 3: a = -∞, b finite
    // x = b - t/(1-t)
    // ============================================================
    if (lower_inf && !upper_inf) {
        auto g = [&](double t) {
            if (t <= 0.0) return f(b);
            if (t >= 1.0) t = 1.0 - eps;

            double x = b - t / (1.0 - t);
            double jac = 1.0 / ((1.0 - t) * (1.0 - t));
            return f(x) * jac;
        };

        return sign * dispatch_finite(g, 0.0, 1.0 - eps, options);
    }

    // ============================================================
    // Case 4: (-∞, +∞)
    // x = tan(pi(t - 0.5))
    // ============================================================
    if (lower_inf && upper_inf) {
        auto g = [&](double t) {
            if (t <= 0.0) t = eps;
            if (t >= 1.0) t = 1.0 - eps;

            double y = M_PI * (t - 0.5);
            double cos_y = std::cos(y);
            double x = std::tan(y);
            double jac = M_PI / (cos_y * cos_y);
            return f(x) * jac;
        };

        return sign * dispatch_finite(g, eps, 1.0 - eps, options);
    }

    throw std::runtime_error("Unsupported integral configuration");
}

}  // namespace numathap