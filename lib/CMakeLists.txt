add_library(data_base 
    data_base.cpp
    string_element.cpp
    list_element.cpp
    set_element.cpp
    geo_element.cpp
    commands.cpp
    commands_string.cpp
    commands_list.cpp
    commands_set.cpp
    commands_geo.cpp
    commands_general.cpp)

target_include_directories(data_base PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})