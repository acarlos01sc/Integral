#include <numathap/integrator.h>

#include <cmath>
#include <stdexcept>

#include "internal/algorithms/gauss_kronrod.h"
#include "internal/algorithms/simpson.h"
#include "internal/evaluator.h"
#include "internal/lexer.h"
#include "internal/parser.h"

namespace numathap {

double integrate(const std::string& function, const std::string& variable,
                 const std::string& lower, const std::string& upper,
                 const IntegratorOptions& options) {
    if (options.abs_tol <= 0.0) {
        throw std::invalid_argument("abs_tol must be a positive number");
    }

    if (options.maxDepth <= 0) {
        throw std::invalid_argument("maxDepth must be positive");
    }

    // Parse the integrand
    Lexer funcLexer(function);
    Parser funcParser(funcLexer);
    auto funcExpr = funcParser.parse();

    // Parse lower limit
    Lexer lowerLexer(lower);
    Parser lowerParser(lowerLexer);
    auto lowerExpr = lowerParser.parse();

    // Parse upper limit
    Lexer upperLexer(upper);
    Parser upperParser(upperLexer);
    auto upperExpr = upperParser.parse();

    Evaluator::Context empty;

    double a = Evaluator::evaluate(*lowerExpr, empty);
    double b = Evaluator::evaluate(*upperExpr, empty);

    if (!std::isfinite(a) || !std::isfinite(b)) {
        throw std::runtime_error("Integration limits are not finite");
    }

    double sign = 1.0;
    if (a > b) {
        std::swap(a, b);
        sign = -1.0;
    }

    // ----------------------------
    // numerical integrand f(x)
    // ----------------------------
    Evaluator::Context ctx;

    auto f = [&](double x) -> double {
        ctx[variable] = x;
        return Evaluator::evaluate(*funcExpr, ctx);
    };

    // --------------------------------
    // Dispatch integration algorithm
    // --------------------------------
    double result = 0.0;

    switch (options.method) {
        case IntegrationMethod::AdaptiveSimpson:
            result = internal::adaptive_simpson(f, a, b, options.abs_tol,
                                                options.maxDepth);
            break;
        case IntegrationMethod::GaussKronrod:
            result = internal::gauss_kronrod(
                f, a, b, options.abs_tol, options.maxDepth, options.gk_rule);
            break;

        default:
            throw std::runtime_error("Integration method not implemented");
    }

    return sign * result;
}

}  // namespace numathap