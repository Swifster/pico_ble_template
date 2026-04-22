add_library(serial_print serial_print.c)

target_include_directories(serial_print PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    serial_print
    pico_stdlib
)