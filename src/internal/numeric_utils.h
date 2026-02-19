#pragma once
#include <cmath>
#include <limits>

namespace numathap::internal {

    inline double finalize_value(double x, double tol) {
        if (!std::isfinite(x))
            return x;

        if (std::abs(x) < tol)
            return 0.0;

        if (std::abs(x) < 10*std::numeric_limits<double>::epsilon())
            return 0.0;

        return x;
    }
}