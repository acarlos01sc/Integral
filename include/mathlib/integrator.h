#pragma once

#include <string>

namespace mathlib {
/**
 * @brief Calculate the definite integral of a scalar function of one variable.
 */
double integrate(const std::string& expression, const std::string& variable,
                    const std::string& lower, const std::string& upper,
                    double precision = 1e-6);

}  // namespace mathlib