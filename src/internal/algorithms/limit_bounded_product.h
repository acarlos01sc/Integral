#pragma once

#include <optional>
#include <string>

#include "numathap/limit.h"
#include "internal/ast.h"

namespace numathap::internal {

std::optional<LimitResult> try_bounded_product(
    const ast::Expr* expr,
    const std::string& variable,
    double point,
    const LimitOptions& options);

}