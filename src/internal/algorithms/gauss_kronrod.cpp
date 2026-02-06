#include "internal/algorithms/gauss_kronrod.h"

#include <cmath>
#include <stdexcept>

namespace numathap::internal {

// ================================================================
// Gauss–Kronrod 7/15 nodes and weights (on [-1, 1])
// Source: QUADPACK / standard tables
// ================================================================

namespace {

constexpr double gk15_x[8] = {
    0.0000000000000000,
    0.2077849550078985,
    0.4058451513773972,
    0.5860872354676911,
    0.7415311855993945,
    0.8648644233597691,
    0.9491079123427585,
    0.9914553711208126
};

constexpr double gk15_wk[8] = {
    0.2094821410847278,
    0.2044329400752989,
    0.1903505780647854,
    0.1690047266392679,
    0.1406532597155259,
    0.1047900103222502,
    0.0630920926299786,
    0.0229353220105292
};

constexpr double gk15_wg[4] = {
    0.4179591836734694,
    0.3818300505051189,
    0.2797053914892766,
    0.1294849661688697
};

// ------------------------------------------------------------
// Compute Gauss–Kronrod 7/15 on [a, b]
// ------------------------------------------------------------
static void gk15_rule(const std::function<double(double)>& f,
                      double a,
                      double b,
                      double& gauss,
                      double& kronrod) {

    const double c = 0.5 * (a + b);
    const double h = 0.5 * (b - a);

    gauss = 0.0;
    kronrod = 0.0;

    const double fc = f(c);
    if (!std::isfinite(fc)) {
        throw std::runtime_error("Function evaluation resulted in NaN or Inf");
    }

    kronrod += gk15_wk[0] * fc;
    gauss   += gk15_wg[0] * fc;

    for (int i = 1; i < 8; ++i) {
        double x = h * gk15_x[i];
        double f1 = f(c - x);
        double f2 = f(c + x);

        if (!std::isfinite(f1) || !std::isfinite(f2)) {
            throw std::runtime_error("Function evaluation resulted in NaN or Inf");
        }

        kronrod += gk15_wk[i] * (f1 + f2);

        // Gauss nodes are subset of Kronrod nodes
        if (i == 2) gauss += gk15_wg[1] * (f1 + f2);
        if (i == 4) gauss += gk15_wg[2] * (f1 + f2);
        if (i == 6) gauss += gk15_wg[3] * (f1 + f2);
    }

    gauss   *= h;
    kronrod *= h;
}

// ------------------------------------------------------------
// Recursive adaptive driver
// ------------------------------------------------------------
static double adaptive_gk15(const std::function<double(double)>& f,
                            double a,
                            double b,
                            double abs_tol,
                            int depth) {

    if (depth <= 0) {
        throw std::runtime_error("Gauss–Kronrod method did not converge");
    }

    double g, k;
    gk15_rule(f, a, b, g, k);

    double err = std::fabs(k - g);

    if (err <= abs_tol) {
        return k;
    }

    double m = 0.5 * (a + b);

    return adaptive_gk15(f, a, m, abs_tol * 0.5, depth - 1) +
           adaptive_gk15(f, m, b, abs_tol * 0.5, depth - 1);
}

}  // anonymous namespace

// ================================================================
// Public interface
// ================================================================
double gauss_kronrod(const std::function<double(double)>& f,
                     double a,
                     double b,
                     double abs_tol,
                     int maxDepth,
                     GaussKronrodRule rule) {

    if (abs_tol <= 0.0) {
        throw std::invalid_argument("abs_tol must be positive");
    }

    if (maxDepth <= 0) {
        throw std::invalid_argument("maxDepth must be positive");
    }

    switch (rule) {
        case GaussKronrodRule::GK15:
            return adaptive_gk15(f, a, b, abs_tol, maxDepth);

        case GaussKronrodRule::GK21:
            throw std::runtime_error("GK21 not implemented yet");

        default:
            throw std::runtime_error("Unknown Gauss–Kronrod rule");
    }
}

}  // namespace numathap::internal