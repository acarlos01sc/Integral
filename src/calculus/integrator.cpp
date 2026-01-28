#include <mathlib/integrator.h>

#include <cmath>
#include <limits>
#include <stdexcept>

#include "evaluator.h"
#include "lexer.h"
#include "parser.h"

namespace mathlib {

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
static double adaptive_simpson(const ast::Expr& expr, const std::string& var,
                               double a, double b, double eps, double whole,
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

    return adaptive_simpson(expr, var, a, c, eps * 0.5, left, depth - 1) +
           adaptive_simpson(expr, var, c, b, eps * 0.5, right, depth - 1);
}

// ----------------------------------------------------------------
// Pubic API: definite integral using adaptive Simpson's method
// ----------------------------------------------------------------
double integrate(const std::string& function, const std::string& variable,
                 const std::string& lower, const std::string& upper,
                 double tolerance) {
    if (tolerance <= 0.0) {
        throw std::invalid_argument("Tolerance must be a positive number");
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

    if (!std::isfinite(a) || !std::isfinite(b)) {
        throw std::runtime_error("Integration limits are not finite");
    }

    double sign = 1.0;
    if (a > b) {
        std::swap(a, b);
        sign = -1.0;
    }

    double whole = simpson(*funcExpr, variable, a, b);

    constexpr int maxDepth = 20;

    double result =
        adaptive_simpson(*funcExpr, variable, a, b, tolerance, whole, maxDepth);

    return sign * result;
}

}  // namespace mathlib