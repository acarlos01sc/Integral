#pragma once

#include <functional>

namespace numathap::internal {

double adaptive_simpson(const std::function<double(double)>& f, double a,
                        double b, double abs_tol, int maxDepth);
}  // namespace numathap::internal