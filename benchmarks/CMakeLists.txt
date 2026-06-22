add_executable(test_bench test_benchmark.cpp)

target_link_libraries(
    test_bench
    PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
)

target_include_directories(test_bench PUBLIC ${PROJECT_SOURCE_DIR})