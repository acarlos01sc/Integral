#pragma once

#include <string>

#include "internal/ast.h"

namespace numathap::internal {

std::string ast_to_string(const ast::Expr* expr);

}