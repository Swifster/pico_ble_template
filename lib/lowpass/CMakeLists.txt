add_library(lowpass lowpass.c)

target_include_directories(lowpass PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    lowpass
    pico_stdlib
)