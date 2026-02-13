#pragma once

#include <functional>
#include <numathap/limit.h>

namespace numathap::internal {

/**
 * @brief Computes the limit using a forward sequence approach.
 *
 * This method is suitable for:
 * - Finite limits (single-sided or one-sided)
 * - Infinite limits
 *
 * @param f             Function f(x) to evaluate.
 * @param point         Point of approach (ignored if infinite).
 * @param options       Limit computation options (tolerance, max iterations, side).
 * @param infinite      True if the limit is at infinity (+∞ or -∞).
 * @param negative_inf  True if the limit is -∞.
 *
 * @return LimitResult containing the computed value and status.
 */
LimitResult limit_forward(
    const std::function<double(double)>& f,
    double point,
    const numathap::LimitOptions& options,
    bool infinite = false,
    bool negative_inf = false);

} // namespace numathap::internal