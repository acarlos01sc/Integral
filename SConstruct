import os
import subprocess

# -------------------------------------------------------------------
# Base environment (C++)
# -------------------------------------------------------------------

env = Environment(
    CXX='clang++',
    CXXFLAGS=[
        '-std=c++20',
        '-Wall',
        '-Wextra',
        '-O2',
        '-g',
        '-fPIC',
    ]
)

BUILD_DIR = 'build'
SRC_DIR = 'src'
INCLUDE_DIR = 'include'
INTERNAL_DIR = os.path.join(SRC_DIR, 'internal')
CALC_DIR = os.path.join(SRC_DIR, 'calculus')
PYTHON_DIR = os.path.join(SRC_DIR, 'python')
TEST_DIR = 'tests'

env.Append(CPPPATH=[INCLUDE_DIR, SRC_DIR])

# -------------------------------------------------------------------
# Core sources
# -------------------------------------------------------------------

internal_sources = (Glob(f'{INTERNAL_DIR}/*.cpp') +
                    Glob(f'{INTERNAL_DIR}/algorithms/*.cpp')
                    )
calculus_sources = Glob(f'{CALC_DIR}/*.cpp')
core_sources = internal_sources + calculus_sources

# -------------------------------------------------------------------
# Static library (C++)
# -------------------------------------------------------------------

lib_static = env.StaticLibrary(
    target=os.path.join(BUILD_DIR, 'numathap'),
    source=core_sources
)

# -------------------------------------------------------------------
# Shared library (C++)
# -------------------------------------------------------------------

lib_shared = env.SharedLibrary(
    target=os.path.join(BUILD_DIR, 'numathap'),
    source=core_sources
)

# -------------------------------------------------------------------
# C++ Tests
# -------------------------------------------------------------------

equation_test = env.Program(
    target=os.path.join(BUILD_DIR, 'equation'),
    source=os.path.join(TEST_DIR, 'equations.cpp'),
    LIBS=[lib_static],
)

eval_test = env.Program(
    target=os.path.join(BUILD_DIR, 'eval_test'),
    source=os.path.join(TEST_DIR, 'eval_test.cpp'),
    LIBS=[lib_static],
)

main_test = env.Program(
    target=os.path.join(BUILD_DIR, 'main'),
    source=os.path.join(TEST_DIR, 'hw.cpp'),
    LIBS=[lib_static],
)

integrator_test = env.Program(
    target=os.path.join(BUILD_DIR, 'integrator_test'),
    source=os.path.join(TEST_DIR, 'integrator_test.cpp'),
    LIBS=[lib_static],
)

# -------------------------------------------------------------------
# Python bindings (pybind11)
# -------------------------------------------------------------------

pybind_includes = subprocess.check_output(
    ['python3', '-m', 'pybind11', '--includes'],
    text=True
).strip().split()

python_ext_suffix = subprocess.check_output(
    ['python3-config', '--extension-suffix'],
    text=True
).strip()

pyenv = env.Clone()
pyenv.Append(CXXFLAGS=pybind_includes)

python_module = pyenv.SharedLibrary(
    target=os.path.join(BUILD_DIR, 'numathap'),
    source=[os.path.join(PYTHON_DIR, 'bindings.cpp')],
    LIBS=[lib_static],
    SHLIBPREFIX='',
    SHLIBSUFFIX=python_ext_suffix,
)

# -------------------------------------------------------------------
# Default targets
# -------------------------------------------------------------------

Default([
    lib_static,
    lib_shared,
    equation_test,
    eval_test,
    main_test,
    integrator_test,
    python_module,
])
