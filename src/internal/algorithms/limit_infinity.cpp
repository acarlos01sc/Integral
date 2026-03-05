#include "limit_infinity.h"

#include <limits>

#include "internal/algorithms/limit_forward.h"

namespace numathap::internal {

LimitResult limit_infinity(
    const std::function<double(double)>& f,
    bool negative_inf,
    const LimitOptions& options) {

    double inf = negative_inf ?
        -std::numeric_limits<double>::infinity() :
         std::numeric_limits<double>::infinity();

    return limit_forward(f, inf, options, true, negative_inf);
}

}