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
    std::cout << "f(x)=cos(x) | f(0) = " << eval("cos(x)",{{"x",0.0}}) << std::endl;
    std::cout << "cos(pi) = " << eval("cos(pi)") << std::endl;
    std::cout << "x=3 e y=2 -> x + y = " << eval("x+y",{{"x",3.0},{"y",2.0}}) << std::endl;
    std::cout << "x=2 e y=3 -> sin(pi/2) + x*y = " << eval("sin(pi/2) + x*y",{{"x",2.0},{"y",3.0}}) << std::endl;
    std::cout << "|-x| quando x=1 -> " << eval("|-x|",{{"x",1.0}}) << std::endl;
    return 0;
}