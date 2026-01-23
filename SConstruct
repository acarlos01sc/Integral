env = Environment(CC="clang", CXX="clang++")
VariantDir('build', 'src', duplicate=False)
env.Program('build/main', Split('build/hw.cpp build/lib/lexer.cpp build/lib/token.cpp'))
