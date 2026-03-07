#pragma once

#include <optional>
#include <string>

#include "internal/ast.h"

namespace numathap::internal {

std::optional<std::string> transform_sqrt_minus_const(
    const ast::Expr* expr);

}