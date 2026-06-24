add_executable(bm_runner benchmark_runner.cpp ${PROJECT_SOURCE_DIR}/lib/thread_pool/thread_pool.cpp)

target_link_libraries(
    bm_runner
    PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
)
