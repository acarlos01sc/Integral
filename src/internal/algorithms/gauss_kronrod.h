#pragma once

#include <functional>
#include <numathap/integrator.h>

namespace numathap::internal {

/**
 * @brief Adaptive Gauss–Kronrod integration.
 *
 * Implements an adaptive Gauss–Kronrod quadrature using
 * a Gauss/Kronrod pair (e.g. 7/15, 10/21).
 *
 * @param f        Function f(x)
 * @param a        Lower integration bound
 * @param b        Upper integration bound
 * @param abs_tol      Absolute error tolerance
 * @param maxDepth Maximum recursion depth
 * @param rule    Kronrod rule order (e.g. 15, 21)
 *
 * @return Approximation of the integral over [a, b]
 */
double gauss_kronrod(const std::function<double(double)>& f, double a, double b,
                     double abs_tol, int maxDepth,
                     GaussKronrodRule rule = GaussKronrodRule::GK15);

}  // namespace numathap::internal