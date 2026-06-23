add_executable(${PROJECT_NAME} main.cpp ${PROJECT_SOURCE_DIR}/lib/thread_pool.cpp)


target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
