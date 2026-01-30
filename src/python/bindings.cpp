#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "numathap/integrator.h"

namespace py = pybind11;

PYBIND11_MODULE(numathap, m) {
    m.doc() = "numathap - Numerical calculus and integration library";

    m.def(
        "integrate",
        &numathap::integrate,
        py::arg("expression"),
        py::arg("variable"),
        py::arg("lower"),
        py::arg("upper"),
        py::arg("precision")=1e-6,
        R"pbdoc(
        Calculate the definite integral of a scalar function of one variable.
        )pbdoc"
    );
}