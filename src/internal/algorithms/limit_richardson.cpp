#include "limit_richardson.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "internal/numeric_utils.h"

namespace numathap::internal {

namespace {

// ------------------------------------------------------------
// Convergence check
// ------------------------------------------------------------
inline bool converged(double prev, double current, double abs_tol,
                      double rel_tol) {
    double diff = std::abs(current - prev);
    double tol = std::max(abs_tol, rel_tol * std::abs(current));
    return diff < tol;
}

// ------------------------------------------------------------
// Heuristic threshold for divergence
// ------------------------------------------------------------
constexpr double LARGE_THRESHOLD = 1e12;

// Machine precision constants for numerical robustness
constexpr double EPS = std::numeric_limits<double>::epsilon();
const double SQRT_EPS = std::sqrt(EPS);
constexpr double TINY = std::numeric_limits<double>::min();
// ------------------------------------------------------------

}  // anonymous namespace

// ----------------------------------------------------------------------
// Richardson Extrapolation Implementation
// ----------------------------------------------------------------------
LimitResult limit_richardson(const std::function<double(double)>& f,
                             double point, const LimitOptions& options) {
    LimitResult result;
    double prev = 0.0;
    double current = 0.0;

    for (int k = 1; k <= options.max_iterations; ++k) {
        static const double SHRINK = 1.0 / std::sqrt(2.0);
        double h = std::pow(SHRINK, k);
        
        double x1, x2;

        // Determine single-sided or bilateral
        if (options.side == LimitSide::Left) {
            x1 = point - h;
            x2 = point - h / 2.0;
        } else {
            x1 = point + h;
            x2 = point + h / 2.0;
        }

        double f1 = f(x1);
        double f2 = f(x2);

        // Reject non-finite evaluations early
        if (!std::isfinite(f1) || !std::isfinite(f2)) {
            result.value = std::numeric_limits<double>::quiet_NaN();
            result.status = LimitStatus::NumericalFailure;
            result.iterations = k;
            return result;
        }

        // Detect catastrophic cancellation or numerical noise
        double scale = std::max({1.0, std::abs(f1), std::abs(f2)});

        if (std::abs(f1) < SQRT_EPS * scale &&
            std::abs(f2) < SQRT_EPS * scale) {
            result.value = finalize_value(f2, options.abs_tolerance);
            result.status = LimitStatus::Converged;
            result.iterations = k;
            return result;
        }

        // Detect underflow region
        if (std::abs(f1) < TINY && std::abs(f2) < TINY) {
            result.value = 0.0;
            result.status = LimitStatus::Converged;
            result.iterations = k;
            return result;
        }

        // Richardson extrapolation (order 1)
        current = 2.0 * f2 - f1;

        // NaN / Inf check
        if (std::isnan(current)) {
            result.value = std::numeric_limits<double>::quiet_NaN();
            result.status = LimitStatus::NumericalFailure;
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
            // Convergence check
            if (converged(prev, current, options.abs_tolerance,
                          options.rel_tolerance)) {
                result.value = finalize_value(current, options.abs_tolerance);
                result.status = LimitStatus::Converged;
                result.iterations = k;
                return result;
            }

            // Oscillation detection (sign alternates)
            if (std::signbit(prev) != std::signbit(current)) {
                result.value = std::numeric_limits<double>::quiet_NaN();
                result.status = LimitStatus::Oscillatory;
                result.iterations = k;
                return result;
            }
        }

        prev = current;
    }

    // Max iterations reached
    result.value = finalize_value(current, options.abs_tolerance);
    result.status = LimitStatus::MaxIterationsReached;
    result.iterations = options.max_iterations;

    return result;
}

}  // namespace numathap::internal