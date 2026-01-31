#include <numathap/integrator.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(numathap, m) {
    m.doc() = "numathap - Numerical calculus and integration library";

    py::enum_<numathap::IntegrationMethod>(m, "IntegrationMethod")
        .value("AdaptiveSimpson", numathap::IntegrationMethod::AdaptiveSimpson)
        .export_values();

    py::class_<numathap::IntegratorOptions>(m,"IntegratorOptions")
        .def(py::init<>())
        .def_readwrite("method",&numathap::IntegratorOptions::method)
        .def_readwrite("precision",&numathap::IntegratorOptions::precision)
        .def_readwrite("max_depth", &numathap::IntegratorOptions::maxDepth);

    m.def("integrate", &numathap::integrate, py::arg("expression"),
          py::arg("variable"), py::arg("lower"), py::arg("upper"),
          py::arg("options") = numathap::IntegratorOptions{},
          R"pbdoc(
        Calculate the definite integral of a scalar function of one variable.
        )pbdoc");
}