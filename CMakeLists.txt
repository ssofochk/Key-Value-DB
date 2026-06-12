cmake_minimum_required(VERSION 3.20)

project(
    app
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(lib)

add_subdirectory(bin)
add_subdirectory(benchmarks)

set(BENCHMARK_ENABLE_TESTING OFF)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF)

add_subdirectory(Bench_util/benchmark)

enable_testing()
add_subdirectory(tests)