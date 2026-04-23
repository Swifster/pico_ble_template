add_library(pico_ble_stack INTERFACE)

target_sources(pico_ble_stack INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/pico_ble_stack.c
)

target_include_directories(pico_ble_stack INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(pico_ble_stack INTERFACE
    pico_stdlib
    pico_btstack_ble
    pico_btstack_cyw43
    pico_cyw43_arch_none
)

pico_btstack_make_gatt_header(pico_ble_stack INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/bluefruit_uart.gatt
    ${PICO_SDK_PATH}/lib/btstack/src/ble/gatt-service
)
