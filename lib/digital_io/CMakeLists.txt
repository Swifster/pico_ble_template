add_library(digital_io digital_io.c)

target_include_directories(digital_io PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    digital_io
    pico_stdlib
    pico_cyw43_arch_none
)