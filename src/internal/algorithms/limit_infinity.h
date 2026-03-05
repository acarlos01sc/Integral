#pragma once

#include <functional>
#include <numathap/limit.h>

namespace numathap::internal {

/**
 * @brief Computes infinite limits numerically.
 *
 * Used when structural analysis is not possible.
 */
LimitResult limit_infinity(
    const std::function<double(double)>& f,
    bool negative_inf,
    const LimitOptions& options);

}