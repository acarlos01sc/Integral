#pragma once

#include <string>

namespace numathap {

/**
 * @brief Available numerical integration algorithms.
 *
 * Specifies which numerical method will be used to evaluate
 * the definite integral.
 */
enum class IntegrationMethod {
    /**
     * Adaptive Simpson's rule.
     *
     * Simple method based on recursive subdivision using Simpson's rule.
     * Works well for smooth functions.
     */
    AdaptiveSimpson,
    /**
     * Adaptive Gauss-Kronrod quadrature.
     *
     * Uses an embedded Gauss-Kronrod pair to estimate the integral and
     * its error, recursively subdividing the interval until the desired
     * tolerance is reached.
     */
    GaussKronrod,
    // ClenshawCurtis,
    // Romberg
};

/**
 * @brief Gauss-Kronrod quadrature rules.
 *
 * Specifies the Gauss-Kronrod node/weight pair used when
 * IntegrationMethod::GaussKronrod is selected.
 */
enum class GaussKronrodRule {
    /** 7-point Gauss / 15-point Kronrod rule (GK15). */
    GK15,
    /** 10-point Gauss / 21-point Kronrod rule (GK21). */
    GK21
};

/**
 * @brief Options controlling numerical integration process.
 *
 * This structure configures the integration algorithm,
 * accuracy requirements and recursion limits.
 */
struct IntegratorOptions {
    /**
     * @brief Integration algorithm to be used.
     *
     * Default is IntegrationMethod::GaussKronrod.
     */
    IntegrationMethod method = IntegrationMethod::GaussKronrod;

    /**
     * @brief Absolute error tolerance.
     *
     * The integration stops when the estimated absolute error
     * is below this value.
     *
     * Must be positive.
     */
    double abs_tol = 1e-6;

    /**
     * @brief Maximum recursion depth.
     *
     * Limits the number of recursive subdivisions to avoid
     * infinite recursion in difficult integrals.
     *
     * Must be positive.
     */
    int maxDepth = 20;

    /**
     * @brief Gauss–Kronrod rule to use.
     *
     * Only relevant when method == IntegrationMethod::GaussKronrod.
     */
    GaussKronrodRule gk_rule = GaussKronrodRule::GK15;
};

/**
 * @brief Compute the definite integral of a scalar function.
 *
 * Evaluates the integral:
 * \f[
 *   \int_{a}^{b} f(x)\,dx
 * \f]
 *
 * where the function is provided as a string expression.
 *
 * @param expression Mathematical expression defining the function.
 *                   Example: "sin(x) / x".
 * @param variable   Name of the independent variable (e.g. "x").
 * @param lower      Lower integration bound (as a string expression).
 * @param upper      Upper integration bound (as a string expression).
 * @param options    Integration configuration options.
 *
 * @return Numerical approximation of the definite integral.
 *
 * @throws std::invalid_argument If parameters are invalid
 *         (e.g. negative tolerance or recursion depth).
 * @throws std::runtime_error If the integration fails to converge
 *         or the function evaluation produces NaN or Inf.
 */
double integrate(const std::string& expression, const std::string& variable,
                 const std::string& lower, const std::string& upper,
                 const IntegratorOptions& options = IntegratorOptions{});

}  // namespace numathap