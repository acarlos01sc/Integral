#include <numathap/integrator.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(numathap, m) {
    m.doc() = R"pbdoc(
numathap
========

Numerical calculus and integration library.

This module provides symbolic expression parsing combined with
high-accuracy numerical integration methods such as:

- Adaptive Simpson
- Gauss–Kronrod (GK15, GK21)

Example
-------
>>> import numathap as nm
>>> nm.integrate("sin(x)", "x", "0", "pi")
2.0

>>> nm.integrate(
...     "exp(-x*x)",
...     "x",
...     "-5",
...     "5",
...     method=nm.IntegrationMethod.GaussKronrod,
...     gk_rule=nm.GaussKronrodRule.GK21,
...     abs_tol=1e-10
... )
1.772453850905516
)pbdoc";

    // -----------------------------
    // Enums
    // -----------------------------
    py::enum_<numathap::IntegrationMethod>(m, "IntegrationMethod")
        .value("AdaptiveSimpson", numathap::IntegrationMethod::AdaptiveSimpson)
        .value("GaussKronrod", numathap::IntegrationMethod::GaussKronrod)
        .export_values();

    py::enum_<numathap::GaussKronrodRule>(m, "GaussKronrodRule")
        .value("GK15", numathap::GaussKronrodRule::GK15)
        .value("GK21", numathap::GaussKronrodRule::GK21)
        .export_values();

    // -----------------------------
    // Python-friendly integrate()
    // -----------------------------
    m.def(
        "integrate",
        [](const std::string& expression, const std::string& variable,
           const std::string& lower, const std::string& upper,
           numathap::IntegrationMethod method, double abs_tol, int max_depth,
           numathap::GaussKronrodRule gk_rule) {
            numathap::IntegratorOptions opt;
            opt.method = method;
            opt.abs_tol = abs_tol;
            opt.maxDepth = max_depth;
            opt.gk_rule = gk_rule;

            return numathap::integrate(expression, variable, lower, upper, opt);
        },
        py::arg("expression"), py::arg("variable"), py::arg("lower"),
        py::arg("upper"),
        py::arg("method") = numathap::IntegrationMethod::GaussKronrod,
        py::arg("abs_tol") = 1e-6, py::arg("max_depth") = 20,
        py::arg("gk_rule") = numathap::GaussKronrodRule::GK15,
        R"pbdoc(
Integrate a scalar function of one variable.

Parameters
----------
expression : str
    Mathematical expression to integrate (e.g. "sin(x)", "exp(-x*x)").
variable : str
    Integration variable name.
lower : str
    Lower integration limit (expression allowed).
upper : str
    Upper integration limit (expression allowed).
method : IntegrationMethod, optional
    Numerical integration method.
    Default is IntegrationMethod.GaussKronrod.
abs_tol : float, optional
    Absolute error tolerance.
max_depth : int, optional
    Maximum recursion depth (used by adaptive algorithms).
gk_rule : GaussKronrodRule, optional
    Gauss–Kronrod quadrature rule (GK15 or GK21).
    Only used when method == IntegrationMethod.GaussKronrod.

Returns
-------
float
    Numerical value of the definite integral.

Notes
-----
- If lower > upper, the sign of the integral is adjusted automatically.
- Expressions are parsed symbolically and evaluated numerically.

Examples
--------
>>> integrate("sin(x)", "x", "0", "pi")
2.0

>>> integrate("1/(1+x*x)", "x", "-10", "10",
...           abs_tol=1e-10)
3.141592653589793
)pbdoc");
}