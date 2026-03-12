#pragma once

#include <optional>
#include <string>

#include "internal/ast.h"

namespace numathap::internal {

// Detecta limites do tipo:
//
// (1 + a/x)^(b*x)
// ((x+a)/x)^(b*x)
// ((k*x+a)/(k*x))^(b*x)
//
// quando x → ±∞.
//
// Retorna o valor do limite se o padrão for reconhecido.
std::optional<double> limit_exp_one_plus_invx(
    const ast::Expr* expr,
    const std::string& var,
    bool infinite,
    bool negative_inf);

}