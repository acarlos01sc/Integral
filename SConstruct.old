env = Environment(CC="clang", CXX="clang++")
VariantDir('build', 'src', duplicate=False)
#env.Program('build/main', Split('build/hw.cpp build/lib/lexer.cpp build/lib/token.cpp build/lib/'))
env.Program('build/main', Split('build/hw.cpp build/lib/lexer.cpp build/lib/parser.cpp'))
env.Program('build/evalteste', Split('build/eval_test.cpp build/lib/lexer.cpp build/lib/parser.cpp build/lib/evaluator.cpp'))
env.Program('build/equation', Split('build/equations.cpp build/lib/lexer.cpp build/lib/parser.cpp build/lib/evaluator.cpp'))

