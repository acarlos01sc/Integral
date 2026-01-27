#include <cassert>
#include <iostream>

#include "../src/internal/lexer.h"
#include "../src/internal/parser.h"
#include "../src/internal/evaluator.h"

using EvaluatorCtx = Evaluator::Context;

// Função auxiliar para reduzir boilerplate
double eval(const std::string& expr, const EvaluatorCtx& ctx = {}) {
    Lexer lexer(expr);
    //auto tokens = lexer.tokenize();

    Parser parser(lexer);
    auto ast = parser.parse();

    return Evaluator::evaluate(*ast, ctx);
}

int main() {

    // ---------------------------
    // Números e operações básicas
    // ---------------------------

    assert(eval("2+3") == 5);
    assert(eval("2+3*4") == 14);
    assert(eval("(2+3)*4") == 20);
    assert(eval("10/2") == 5);

    // ---------------------------
    // Operadores unários
    // ---------------------------

    assert(eval("-3") == -3);
    assert(eval("+3") == 3);
    assert(eval("|-3|") == 3);
    assert(eval("|3|") == 3);

    // ---------------------------
    // Variáveis
    // ---------------------------

    assert(eval("x", {{"x", 2.0}}) == 2.0);
    assert(eval("x + 1", {{"x", 2.0}}) == 3.0);
    assert(eval("x*x", {{"x", 3.0}}) == 9.0);

    // ---------------------------
    // Expressões mais complexas
    // ---------------------------

    assert(eval("|x - 2|", {{"x", 5.0}}) == 3.0);
    assert(eval("|x| + 1", {{"x", -2.0}}) == 3.0);

    // ---------------------------
    // Potência
    // ---------------------------

    assert(eval("2^3") == 8);
    assert(eval("x^2", {{"x", 4.0}}) == 16);
    assert(eval("|x|^2", {{"x", -3.0}}) == 9);

    // ---------------------------
    // Combinação geral
    // ---------------------------

    double r = eval("x^2 + 2*x + 1", {{"x", 3.0}});
    assert(r == 16.0);

    std::cout << "Todos os testes do evaluator passaram com sucesso!\n";
    return 0;
}