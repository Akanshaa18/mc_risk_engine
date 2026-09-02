add_library(risk_core STATIC
    src/pricer.cpp
    src/mc_engine.cpp
    src/risk_metrics.cpp
)
target_include_directories(risk_core PUBLIC include)

find_package(pybind11 REQUIRED)
pybind11_add_module(risk_engine bindings/bindings.cpp)
target_link_libraries(risk_engine PRIVATE risk_core)
add_executable(test_pricer tests/test_pricer.cpp)
target_link_libraries(test_pricer PRIVATE risk_core)
add_executable(mc_engine_check src/mc_engine.cpp src/pricer.cpp)
target_link_libraries(mc_engine_check PRIVATE risk_core)