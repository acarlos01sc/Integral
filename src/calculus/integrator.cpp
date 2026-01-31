#include <numathap/integrator.h>

#include <cmath>
#include <stdexcept>

#include "internal/evaluator.h"
#include "internal/lexer.h"
#include "internal/parser.h"
#include "internal/simpson.h"

namespace numathap {

double integrate(const std::string& function, const std::string& variable,
                 const std::string& lower, const std::string& upper,
                 const IntegratorOptions& options) {
    if (options.precision <= 0.0) {
        throw std::invalid_argument("Tolerance must be a positive number");
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

    double result = 0.0;

    switch (options.method) {
        case IntegrationMethod::AdaptiveSimpson:
            result = internal::adaptive_simpson(
                *funcExpr, variable, a, b, options.precision, options.maxDepth);
            break;
        
        default:
                throw std::runtime_error("Integration method not implemented");
    }

    return sign * result;
}

}  // namespace numathap