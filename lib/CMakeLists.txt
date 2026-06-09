add_library(data_base 
    string_element.cpp
    list_element.cpp
    set_element.cpp
    geo_element.cpp
    commands.cpp)

target_include_directories(data_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})