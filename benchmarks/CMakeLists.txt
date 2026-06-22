add_executable(seq_loop sequential_loop_bm.cpp)

target_link_libraries(
    seq_loop
    PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
)

target_include_directories(seq_loop PUBLIC ${PROJECT_SOURCE_DIR})