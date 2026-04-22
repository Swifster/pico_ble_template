set(PROJECT_NAME pico_debug_uart)

add_library(${PROJECT_NAME}
    pico_debug_uart.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    #pico_cyw43_arch_none
    hardware_adc
    hardware_gpio
)

