#include <numathap/integrator.h>
#include <numathap/limit.h>           // include do limit
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(numathap, m) {
    m.doc() = R"pbdoc(
numathap
========

Numerical calculus and integration library.
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

    py::enum_<numathap::LimitSide>(m, "LimitSide")
        .value("Both", numathap::LimitSide::Both)
        .value("Left", numathap::LimitSide::Left)
        .value("Right", numathap::LimitSide::Right)
        .export_values();

    py::enum_<numathap::LimitMethod>(m, "LimitMethod")
        .value("Auto", numathap::LimitMethod::Auto)
        .value("Forward", numathap::LimitMethod::Forward)
        .value("Richardson", numathap::LimitMethod::Richardson)
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
)pbdoc");

    // -----------------------------
    // Python-friendly limit()
    // -----------------------------
    m.def(
        "limit",
        [](const std::string& expression, const std::string& variable,
           const std::string& value,
           numathap::LimitSide side,
           numathap::LimitMethod method,
           double abs_tol,
           double rel_tol,
           int max_iterations) {

            numathap::LimitOptions options;
            options.side = side;
            options.method = method;
            options.abs_tolerance = abs_tol;
            options.rel_tolerance = rel_tol;
            options.max_iterations = max_iterations;

            numathap::LimitResult res = numathap::limit(expression, variable, value, options);

            // Retorna um dict Python-friendly
            py::dict pyres;
            pyres["value"] = res.value;
            pyres["status"] = static_cast<int>(res.status); // enum como int
            pyres["iterations"] = res.iterations;
            return pyres;
        },
        py::arg("expression"), py::arg("variable"), py::arg("value"),
        py::arg("side") = numathap::LimitSide::Both,
        py::arg("method") = numathap::LimitMethod::Auto,
        py::arg("abs_tol") = 1e-8,
        py::arg("rel_tol") = 1e-8,
        py::arg("max_iterations") = 30,
        R"pbdoc(
Compute the limit of a function as a variable approaches a value.

Parameters
----------
expression : str
    Mathematical expression (e.g. "sin(x)/x").
variable : str
    Variable name.
value : str
    Point of approach ("0", "1.5", "inf", "-inf").
side : LimitSide, optional
    Direction of approach (Both, Left, Right).
method : LimitMethod, optional
    Numerical method (Auto, Forward, Richardson).
abs_tol : float, optional
    Absolute tolerance.
rel_tol : float, optional
    Relative tolerance.
max_iterations : int, optional
    Maximum number of refinement iterations.

Returns
-------
dict
    Dictionary with keys: "value", "status", "iterations".
)pbdoc");
}