add_library(bandpass bandpass.c)

target_include_directories(bandpass PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    bandpass
    pico_stdlib
)