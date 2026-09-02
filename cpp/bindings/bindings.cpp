#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "instrument.h"
#include "mc_engine.h"

namespace py = pybind11;

PYBIND11_MODULE(risk_engine, m){
    //enum
    py::enum_<OptionType>(m, "OptionType")
        .value("Call", OptionType::Call)
        .value("Put", OptionType::Put);

    py::class_<Instrument>(m, "Instrument")
        .def(py::init<>())
        .def_readwrite("underlying", &Instrument::underlying)
        .def_readwrite("isOption", &Instrument::isOption)
        .def_readwrite("type", &Instrument::type)
        .def_readwrite("strike", &Instrument::strike)
        .def_readwrite("maturity", &Instrument::maturity)
        .def_readwrite("quantity", &Instrument::quantity);

    py::class_<MC_Engine>(m, "MC_Engine")
        .def(py::init<>())
        .def_readwrite("s1_curr", &MC_Engine::s1_curr)
        .def_readwrite("s2_curr", &MC_Engine::s2_curr)
        .def_readwrite("drift", &MC_Engine::drift)
        .def_readwrite("r", &MC_Engine::r)
        .def_readwrite("vol1", &MC_Engine::vol1)
        .def_readwrite("vol2", &MC_Engine::vol2)
        .def_readwrite("del_t", &MC_Engine::del_t)
        .def_readwrite("N", &MC_Engine::N)
        .def_readwrite("cholesky_matrix", &MC_Engine::cholesky_matrix)
        .def_readwrite("portfolio", &MC_Engine::portfolio);

    py::class_<MC_returns>(m,"MC_returns")
        .def(py::init<>())
        .def_readwrite("pnl", &MC_returns::pnl)
        .def_readwrite("var95", &MC_returns::var95)
        .def_readwrite("var99", &MC_returns::var99)
        .def_readwrite("cvar95", &MC_returns::cvar95)
        .def_readwrite("cvar99", &MC_returns::cvar99);
    
    m.def("riskEngine", &riskEngine,"Run the Monte Carlo risk engine and return VaR/CVaR");
}