#pragma once

#include <functional>
#include <numathap/limit.h>

namespace numathap::internal {

/**
 * @brief Computes the limit using Richardson extrapolation.
 *
 * Richardson extrapolation accelerates convergence of a sequence
 * by combining function evaluations at decreasing step sizes.
 *
 * Suitable only for finite limits.
 *
 * @param f       Function f(x) to evaluate.
 * @param point   Point of approach (finite).
 * @param options Limit computation options (tolerance, max iterations, side).
 *
 * @return LimitResult containing the computed value and status.
 */
LimitResult limit_richardson(
    const std::function<double(double)>& f,
    double point,
    const LimitOptions& options);

} // namespace numathap::internal