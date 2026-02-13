#include "limit_forward.h"

#include <cmath>
#include <limits>
#include <algorithm>

namespace numathap::internal {

namespace {

// ------------------------------------------------------------
// Convergence check
// ------------------------------------------------------------
inline bool converged(double prev, double current,
                      double abs_tol, double rel_tol) {
    double diff = std::abs(current - prev);
    double tol = std::max(abs_tol, rel_tol * std::abs(current));
    return diff < tol;
}

// ------------------------------------------------------------
// Heuristic threshold for divergence
// ------------------------------------------------------------
constexpr double LARGE_THRESHOLD = 1e12;

} // anonymous namespace

// ----------------------------------------------------------------------
// Forward Sequence Implementation
// ----------------------------------------------------------------------
LimitResult limit_forward(
    const std::function<double(double)>& f,
    double point,
    const numathap::LimitOptions& options,
    bool infinite,
    bool negative_inf) {

    LimitResult result;
    double prev = 0.0;
    double current = 0.0;

    for (int k = 1; k <= options.max_iterations; ++k) {

        // Step size: h → 0
        double h = std::pow(0.5, k);
        double x;

        if (infinite) {
            // Transform x = ±1/h → ±∞
            x = negative_inf ? -1.0 / h : 1.0 / h;
        } else {
            // Single-sided or default
            if (options.side == LimitSide::Left)
                x = point - h;
            else
                x = point + h;
        }

        // Evaluate function safely
        current = f(x);

        // Check for NaN / Inf
        if (std::isnan(current)) {
            result.value = std::numeric_limits<double>::quiet_NaN();
            result.status = LimitStatus::Undefined;
            result.iterations = k;
            return result;
        }

        if (std::isinf(current) || std::abs(current) > LARGE_THRESHOLD) {
            result.value = current;
            result.status = LimitStatus::Divergent;
            result.iterations = k;
            return result;
        }

        if (k > 1) {
            // Check convergence
            if (converged(prev, current,
                          options.abs_tolerance,
                          options.rel_tolerance)) {
                result.value = current;
                result.status = LimitStatus::Converged;
                result.iterations = k;
                return result;
            }

            // Simple oscillation detection: sign alternates
            if (std::signbit(prev) != std::signbit(current)) {
                result.value = std::numeric_limits<double>::quiet_NaN();
                result.status = LimitStatus::Oscillatory;
                result.iterations = k;
                return result;
            }
        }

        prev = current;
    }

    // Maximum iterations reached
    result.value = current;
    result.status = LimitStatus::MaxIterationsReached;
    result.iterations = options.max_iterations;

    return result;
}

} // namespace numathap::internal