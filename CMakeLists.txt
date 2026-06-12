cmake_minimum_required(VERSION 3.20)

project(
    app
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(lib)

add_subdirectory(lib)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)