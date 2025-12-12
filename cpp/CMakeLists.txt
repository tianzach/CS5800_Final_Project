cmake_minimum_required(VERSION 3.10)
project(CCL_Benchmark)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler optimizations
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_BUILD_TYPE Release)

# Source files
add_library(ccl_lib
    dsu_2pass.cpp
    algorithms.cpp
)

# Benchmark executable
add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark ccl_lib)

# Test executable
add_executable(test_algorithms test_algorithms.cpp)
target_link_libraries(test_algorithms ccl_lib)

# Optional: Python bindings (if pybind11 is available)
# find_package(pybind11 QUIET)
# if(pybind11_FOUND)
#     pybind11_add_module(ccl_pybind dsu_2pass.cpp algorithms.cpp)
# endif()

