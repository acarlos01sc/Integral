#include <numathap/integrator.h>

#include <chrono>
#include <iomanip>
#include <iostream>

using clock_type = std::chrono::high_resolution_clock;

double run_test(const std::string& name,
                numathap::IntegrationMethod method,
                const std::string& expr,
                const std::string& var,
                const std::string& a,
                const std::string& b,
                numathap::IntegratorOptions options) {

    options.method = method;

    auto t0 = clock_type::now();
    double result = numathap::integrate(expr, var, a, b, options);
    auto t1 = clock_type::now();

    std::chrono::duration<double, std::milli> elapsed = t1 - t0;

    std::cout << std::left << std::setw(20) << name
              << " result = " << std::setprecision(12) << result
              << " | time = " << elapsed.count() << " ms\n";

    return result;
}

int main() {
    const std::string expr = "exp(-x*x)";
    const std::string var  = "x";
    const std::string a    = "-5";
    const std::string b    = "5";

    numathap::IntegratorOptions options;
    options.abs_tol  = 1e-10;
    options.maxDepth = 15;

    std::cout << "Testing integral of exp(-x^2) on [-5, 5]\n\n";

    double simpson = run_test(
        "Adaptive Simpson",
        numathap::IntegrationMethod::AdaptiveSimpson,
        expr, var, a, b, options
    );

    double gk = run_test(
        "Gauss–Kronrod",
        numathap::IntegrationMethod::GaussKronrod,
        expr, var, a, b, options
    );

    std::cout << "\nDifference |GK - Simpson| = "
              << std::abs(gk - simpson) << "\n";

    return 0;
}