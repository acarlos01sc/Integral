#include <iostream>
#include "lib/evaluator.h"
#include "lib/lexer.h"
#include "lib/parser.h"

using EvaluatorCtx = Evaluator::Context;

double eval(const std::string& expr, const EvaluatorCtx& ctx = {}) {
    Lexer lexer(expr);
    //auto tokens = lexer.tokenize();

    Parser parser(lexer);
    auto ast = parser.parse();

    return Evaluator::evaluate(*ast, ctx);
}

int main() {
    std::cout << "2+3 = " << eval("2+3") << std::endl;
    std::cout << "f(x)=x^3 | f(3) = " << eval("x^3",{{"x",3.0}}) << std::endl;
    return 0;
}