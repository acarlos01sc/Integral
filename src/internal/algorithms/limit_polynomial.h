#pragma once

#include <limits>
#include <string>

//#include "internal/numeric_utils.h"

namespace ast {
struct Expr;
}


namespace numathap {
namespace internal {

struct LeadingTerm {
    double coefficient;
    int degree;
    bool valid;
};


// Sentinel value indicating that polynomial degree detection failed
constexpr int INVALID_DEGREE = std::numeric_limits<int>::min();

// Utility for returning an invalid term
LeadingTerm invalid_term();

// ---------------------------------------------------------------------------
// detect_degree
// ---------------------------------------------------------------------------
// Attempts to determine the polynomial degree of an expression with respect
// to a given variable using structural analysis of the AST.
//
// Supported structures:
//
//   - constants
//   - variables
//   - unary + and -
//   - +, -, *, /
//   - powers with integer exponent
//
// If the expression cannot be safely interpreted as a polynomial,
// the function returns INVALID_DEGREE.
//
int detect_degree(const ast::Expr* node, const std::string& variable);

// ---------------------------------------------------------------------------
// extract_leading_term
// ---------------------------------------------------------------------------
// Extracts the highest-degree term of an expression with respect to a
// variable using structural analysis of the AST.
//
// This function is used to compute limits of rational functions such as:
//
//      (4*x^3 - 2*x^2 + 1) / (3*x^3 - 5)  as x -> ±∞
//
// If the expression cannot be interpreted as a polynomial,
// the returned LeadingTerm will have `valid = false`.
//
LeadingTerm extract_leading_term(const ast::Expr* node,
                                 const std::string& variable);


}  // namespace internal
}  // namespace numathap