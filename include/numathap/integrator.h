#pragma once

#include <string>

namespace numathap {

/**
 * @brief Available numerical integration algorithms
 */
enum class IntegrationMethod {
    AdaptiveSimpson,
    // GaussLegendre,
    // Romberg
};

/**
 * @brief Options controlling numerical integration
 */
struct IntegratorOptions {
    IntegrationMethod method = IntegrationMethod::AdaptiveSimpson;
    double precision = 1e-6;
    int maxDepth = 20;
};

/**
 * @brief Calculate the definite integral of a scalar function of one variable.
 */
double
integrate(const std::string& expression, const std::string& variable,
          const std::string& lower, const std::string& upper,
          const IntegratorOptions& options = IntegratorOptions{});

}  // namespace numathap