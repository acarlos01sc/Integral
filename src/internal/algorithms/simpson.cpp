#include "internal/algorithms/simpson.h"

#include <cmath>
#include <functional>
#include <stdexcept>

#include "internal/evaluator.h"

namespace numathap::internal {

// ----------------------------------------------------------------
// Simple Simpson's rule on interval [a,b]
// ----------------------------------------------------------------
static double simpson(const std::function<double(double)>& f, double a,
                      double b) {
    double c = 0.5 * (a + b);
    double fa = f(a);
    double fb = f(b);
    double fc = f(c);

    if (!std::isfinite(fa) || !std::isfinite(fb) || !std::isfinite(fc)) {
        throw std::runtime_error("function evaluation resulted in NaN or Inf");
    }

    return ((b - a) / 6.0) * (fa + (4.0 * fc) + fb);
}

// ----------------------------------------------------------------
// Recursive adaptive Simpson's method
// ----------------------------------------------------------------
static double adaptive_simpson_recursive(const std::function<double(double)>& f,
                                         double a, double b, double eps,
                                         double whole, int depth) {
    if (depth <= 0) {
        throw std::runtime_error("Adaptive Simpson method did not converge");
    }
    double c = 0.5 * (a + b);
    double left = simpson(f, a, c);
    double right = simpson(f, c, b);

    double delta = left + right - whole;

    if (std::fabs(delta) <= 15.0 * eps) {
        return left + right + delta / 15.0;
    }

    return adaptive_simpson_recursive(f, a, c, eps * 0.5, left, depth - 1) +
           adaptive_simpson_recursive(f, c, b, eps * 0.5, right, depth - 1);
}

double adaptive_simpson(const std::function<double(double)>& f, double a,
                        double b, double abs_tol, int maxDepth) {
    if (abs_tol<=0.0) {
        throw std::invalid_argument("abs_tol must be positive");
    }
    if (maxDepth <= 0) {
        throw std::invalid_argument("maxDepth must be positive");
    }
    double whole = simpson(f,a,b);
    return adaptive_simpson_recursive(f,a,b,abs_tol,whole,maxDepth);
}

}  // namespace numathap::internal