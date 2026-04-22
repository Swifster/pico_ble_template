add_library(SEGGER_RTT
    SEGGER_RTT.c
    SEGGER_RTT_printf.c
)

target_include_directories(SEGGER_RTT PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    SEGGER_RTT
    pico_stdlib
)
