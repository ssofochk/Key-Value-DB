add_executable(bm_runner benchmark_runner.cpp)

target_link_libraries(
    bm_runner
    PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
)
