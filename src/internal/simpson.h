#pragma once

#include <string>
#include "internal/ast.h"

namespace numathap::internal {
    
    double adaptive_simpson(const ast::Expr& expr,
                            const std::string& var,
                            double a,
                            double b,
                            double eps,
                            int maxDepth);
}