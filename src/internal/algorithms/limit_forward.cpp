#include "limit_forward.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <vector>

#include "internal/numeric_utils.h"

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
            double a = e[i];
            double b = e[i + 1];

            if (!std::isfinite(a) || !std::isfinite(b)) {
                e[i] = std::numeric_limits<double>::quiet_NaN();
                continue;
            }

            double diff = b - a;

            if (std::abs(diff) < std::numeric_limits<double>::epsilon()) {
                e[i] = std::numeric_limits<double>::quiet_NaN();
            } else {
                e[i] = b + 1.0 / diff;
            }
        }
    }

    return e[0];
}

// ============================================================
// Hybrid convergence check
// ============================================================
inline bool converged(double prev, double current, double abs_tol,
                      double rel_tol) {
    double diff = std::abs(current - prev);
    double tol = std::max(abs_tol, rel_tol * std::abs(current));
    return diff < tol;
}

}  // anonymous namespace

// ============================================================
// MAIN LIMIT ALGORITHM
// ============================================================
LimitResult limit_forward(const std::function<double(double)>& f, double point,
                          const LimitOptions& options, bool infinite,
                          bool negative_inf) {
    LimitResult result;

    std::vector<double> seq;
    seq.reserve(options.max_iterations);

    double h = 0.5;
    double last_accel = std::numeric_limits<double>::quiet_NaN();

    int growth_streak = 0;
    int sign_changes = 0;
    int positive_count = 0;
    int negative_count = 0;

    double previous_abs = 0.0;
    double previous_fx = 0.0;
    bool first_value = true;

    for (int k = 1; k <= options.max_iterations; ++k) {
        double x;

        // --------------------------------------------------------
        // Infinite limit transform (tan mapping)
        // --------------------------------------------------------
        if (infinite) {
            double t = (M_PI / 2.0) * (1.0 - h);
            x = std::tan(t);

            if (negative_inf) x = -x;

        } else {
            if (options.side == LimitSide::Left)
                x = point - h;
            else
                x = point + h;
        }

        double fx = f(x);

        if (std::isnan(fx)) {
            return {std::numeric_limits<double>::quiet_NaN(),
                    LimitStatus::NumericalFailure, k};
        }

        if (!std::isfinite(fx)) {
            if (positive_count > 3)
                return {std::numeric_limits<double>::infinity(),
                        LimitStatus::Divergent, k};

            if (negative_count > 3)
                return {-std::numeric_limits<double>::infinity(),
                        LimitStatus::Divergent, k};

            h *= 0.5;
            continue;
        }

        seq.push_back(fx);

        double current_abs = std::abs(fx);

        // --------------------------------------------------------
        // Envelope-based zero detection (robust oscillatory case)
        // --------------------------------------------------------
        if (!infinite) {
            double distance = std::abs(x - point);

            if (distance < std::sqrt(options.abs_tolerance) &&
                current_abs < options.abs_tolerance * 10) {
                result.value = 0.0;
                result.status = LimitStatus::Converged;
                result.iterations = k;
                return result;
            }
        }

        // --------------------------------------------------------
        // Asymptotic growth detection
        // --------------------------------------------------------
        if (!first_value) {
            if (previous_abs > 0.0) {
                double ratio = current_abs / previous_abs;

                // Detect real divergence only if magnitude is exploding
                if (ratio > 1.0 + options.rel_tolerance &&
                    current_abs > previous_abs && current_abs > 1.0) {
                    growth_streak++;
                } else {
                    growth_streak = 0;
                }
            }

            if (fx * previous_fx < 0.0) sign_changes++;
        }

        if (fx > 0.0) positive_count++;
        if (fx < 0.0) negative_count++;

        previous_abs = current_abs;
        previous_fx = fx;
        first_value = false;

        // --------------------------------------------------------
        // Divergence classification
        // --------------------------------------------------------
        if (growth_streak >= 5) {
            if (positive_count == static_cast<int>(seq.size())) {
                result.value = std::numeric_limits<double>::infinity();
                result.status = LimitStatus::Divergent;
                result.iterations = k;
                return result;
            }

            if (negative_count == static_cast<int>(seq.size())) {
                result.value = -std::numeric_limits<double>::infinity();
                result.status = LimitStatus::Divergent;
                result.iterations = k;
                return result;
            }

            if (sign_changes > 2) {
                // NEW: check envelope decay
                if (current_abs < options.abs_tolerance * 10) {
                    result.value = 0.0;
                    result.status = LimitStatus::Converged;
                    result.iterations = k;
                    return result;
                }

                result.value = std::numeric_limits<double>::quiet_NaN();
                result.status = LimitStatus::Undefined;
                result.iterations = k;
                return result;
            }
        }

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
                        converged(last_accel, accel, options.abs_tolerance,
                                  options.rel_tolerance)) {
                        result.value =
                            finalize_value(accel, options.abs_tolerance);
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
        result.value = finalize_value(seq.back(), options.abs_tolerance);
        result.status = LimitStatus::MaxIterationsReached;
        result.iterations = options.max_iterations;
        return result;
    }

    result.value = std::numeric_limits<double>::quiet_NaN();
    result.status = LimitStatus::Undefined;
    result.iterations = options.max_iterations;

    return result;
}

}  // namespace numathap::internal