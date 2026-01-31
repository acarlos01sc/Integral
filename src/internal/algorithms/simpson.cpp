#include "internal/simpson.h"

#include <cmath>
#include <stdexcept>

#include "internal/evaluator.h"

namespace numathap::internal {

// ----------------------------------------------------------------
// Helper function: evaluate f(x) using Evaluator and a context
// ----------------------------------------------------------------
static double eval_function(const ast::Expr& expr, const std::string& var,
                            double x) {
    Evaluator::Context ctx;
    ctx[var] = x;

    double value = Evaluator::evaluate(expr, ctx);

    // Basic numerical sanity check
    if (!std::isfinite(value)) {
        throw std::runtime_error("function evaluation in NaN or Inf");
    }

    return value;
}

// ----------------------------------------------------------------
// Simple Simpson's rule on interval [a,b]
// ----------------------------------------------------------------
static double simpson(const ast::Expr& expr, const std::string& var, double a,
                      double b) {
    double c = 0.5 * (a + b);
    double fa = eval_function(expr, var, a);
    double fb = eval_function(expr, var, b);
    double fc = eval_function(expr, var, c);

    return ((b - a) / 6.0) * (fa + (4.0 * fc) + fb);
}

// ----------------------------------------------------------------
// Recursive adaptive Simpson's method
// ----------------------------------------------------------------
static double adaptive_simpson_recursive(const ast::Expr& expr,
                                         const std::string& var, double a,
                                         double b, double eps, double whole,
                                         int depth) {
    if (depth <= 0) {
        throw std::runtime_error("Adaptive Simpson method did not converge");
    }
    double c = 0.5 * (a + b);
    double left = simpson(expr, var, a, c);
    double right = simpson(expr, var, c, b);

    double delta = left + right - whole;

    if (std::fabs(delta) <= 15.0 * eps) {
        return left + right + delta / 15.0;
    }

    return adaptive_simpson_recursive(expr, var, a, c, eps * 0.5, left,
                                      depth - 1) +
           adaptive_simpson_recursive(expr, var, c, b, eps * 0.5, right,
                                      depth - 1);
}

double adaptive_simpson(const ast::Expr& expr, const std::string& var, double a,
                        double b, double eps, int maxDepth) {
    double whole = simpson(expr, var, a, b);
    return adaptive_simpson_recursive(expr, var, a, b, eps, whole, maxDepth);
}

}  // namespace numathap::internal