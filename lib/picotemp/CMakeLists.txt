add_library(picotemp picotemp.c)

target_include_directories(picotemp PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    picotemp
    pico_stdlib
    #pico_cyw43_arch_none
    hardware_adc
)