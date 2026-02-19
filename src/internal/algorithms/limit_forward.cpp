#include <cmath>
#include <limits>
#include <vector>
#include <algorithm>
#include "internal/numeric_utils.h"

#include "limit_forward.h"

namespace numathap::internal {

namespace {

// ============================================================
// Wynn epsilon acceleration
// ============================================================
double wynn_epsilon(const std::vector<double>& seq) {

    size_t n = seq.size();
    if (n < 3) return seq.back();

    std::vector<double> e(seq);

    for (size_t k = 1; k < n; ++k) {
        for (size_t i = 0; i < n - k; ++i) {

            double diff = e[i + 1] - e[i];

            if (std::abs(diff) < std::numeric_limits<double>::epsilon()) {
                e[i] = std::numeric_limits<double>::infinity();
            } else {
                e[i] = e[i + 1] + 1.0 / diff;
            }
        }
    }

    return e[0];
}

// ============================================================
// Hybrid convergence check
// ============================================================
inline bool converged(double prev,
                      double current,
                      double abs_tol,
                      double rel_tol) {

    double diff = std::abs(current - prev);
    double tol = std::max(abs_tol, rel_tol * std::abs(current));
    return diff < tol;
}

} // anonymous namespace

// ============================================================
// MAIN LIMIT ALGORITHM
// ============================================================
LimitResult limit_forward(
    const std::function<double(double)>& f,
    double point,
    const LimitOptions& options,
    bool infinite,
    bool negative_inf)
{
    LimitResult result;

    std::vector<double> seq;
    seq.reserve(options.max_iterations);

    double h = 0.5;
    double last_accel = std::numeric_limits<double>::quiet_NaN();

    for (int k = 1; k <= options.max_iterations; ++k) {

        double x;

        // --------------------------------------------------------
        // Infinite limit transform (tan mapping)
        // --------------------------------------------------------
        if (infinite) {

            double t = (M_PI / 2.0) * (1.0 - h);
            x = std::tan(t);

            if (negative_inf)
                x = -x;

        } else {

            if (options.side == LimitSide::Left)
                x = point - h;
            else
                x = point + h;
        }

        double fx = f(x);

        if (!std::isfinite(fx)) {
            h *= 0.5;
            continue;
        }

        seq.push_back(fx);

        // --------------------------------------------------------
        // Apply Wynn when enough data
        // --------------------------------------------------------
        if (seq.size() >= 3) {

            double accel = wynn_epsilon(seq);

            if (std::isfinite(accel)) {

                // ⭐ Require asymptotic regime for infinite limits
                bool asymptotic_regime =
                    !infinite || (h < std::sqrt(options.abs_tolerance));

                if (asymptotic_regime) {

                    // ⭐ Zero limit criterion
                    if (std::abs(accel) < options.abs_tolerance) {
                        result.value = accel;
                        result.status = LimitStatus::Converged;
                        result.iterations = k;
                        return result;
                    }

                    // ⭐ Standard convergence
                    if (std::isfinite(last_accel) &&
                        converged(last_accel, accel,
                                  options.abs_tolerance,
                                  options.rel_tolerance)) {

                        result.value = finalize_value(accel,options.abs_tolerance);
                        result.status = LimitStatus::Converged;
                        result.iterations = k;
                        return result;
                    }
                }

                last_accel = accel;
            }
        }

        h *= 0.5;
    }

    // --------------------------------------------------------
    // Fallback
    // --------------------------------------------------------
    if (!seq.empty()) {
        result.value = finalize_value(seq.back(),options.abs_tolerance);
        result.status = LimitStatus::MaxIterationsReached;
        result.iterations = options.max_iterations;
        return result;
    }

    result.value = std::numeric_limits<double>::quiet_NaN();
    result.status = LimitStatus::Undefined;
    result.iterations = options.max_iterations;

    return result;
}

} // namespace numathap::internal