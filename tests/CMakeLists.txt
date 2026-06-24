include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/releases/download/v1.16.0/googletest-1.16.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    app-tests
    cache_tests.cpp
    ${PROJECT_SOURCE_DIR}/lib/thread_pool/thread_pool.cpp
)

target_include_directories(
    app-tests
    PUBLIC
    ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(
    app-tests
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(app-tests)