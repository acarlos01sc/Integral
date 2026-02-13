#pragma once

#include <string>

namespace numathap {

/**
 * @brief Direction from which the limit is evaluated.
 */
enum class LimitSide {
    Both,  ///< Two-sided limit
    Left,  ///< x → a⁻
    Right  ///< x → a⁺
};

/**
 * @brief Numerical method used to compute the limit.
 */
enum class LimitMethod {
    Auto,        ///< Automatically choose the best method
    Richardson,  ///< Richardson extrapolation
    Forward      ///< Forward sequence h → 0
};

/**
 * @brief Configuration options for limit computation.
 */
struct LimitOptions {
    /// Direction of approach
    LimitSide side = LimitSide::Both;

    /// Absolute tolerance for convergence
    double abs_tolerance = 1e-8;

    /// Relative tolerance for convergence
    double rel_tolerance = 1e-8;

    /// Maximum number of refinement iterations
    int max_iterations = 20;

    /// Numerical method to use
    LimitMethod method = LimitMethod::Auto;
};

enum class LimitStatus {
    Converged,
    Divergent,
    Oscillatory,
    Undefined,
    MaxIterationsReached
};

/**
 * @brief Result structure returned by limit().
 */
struct LimitResult {
    /// Computed limit value (if successful)
    double value = 0.0;

    /// Status of the limit computation
    LimitStatus status = LimitStatus::Undefined;

    /// Number of iterations used
    int iterations = 0;
};

/**
 * @brief Computes the limit of a function as a variable approaches a value.
 *
 * @param expression  Mathematical expression as string.
 * @param variable  Variable name.
 * @param value     Point of approach ("0", "1.5", "inf", "-inf").
 * @param options   Optional configuration parameters.
 *
 * @return LimitResult containing value and convergence status.
 */
LimitResult limit(const std::string& expression, const std::string& variable,
                  const std::string& value, const LimitOptions& options = {});

}  // namespace numathap