add_library(highpass highpass.c)

target_include_directories(highpass PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    highpass
    pico_stdlib
)