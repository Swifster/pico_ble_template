#Set name for the generated firmware output .elf .hex
set(FIRMWARE_NAME pico_ble_template)

# Create the executable
add_executable(${FIRMWARE_NAME}
    main.c
)

# Set the output directory for the executable
set_target_properties(${FIRMWARE_NAME}  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"  # Adjust as needed
    ARCHIVE_OUTPUT_DIRECTORY "${PICO_UF2_OUTPUT_DIR}"
)


target_link_libraries(${FIRMWARE_NAME} 
    pico_stdlib
    pico_ble_stack
)

target_include_directories(${FIRMWARE_NAME}  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Enable standard I/O over USB/UART
pico_enable_stdio_usb(${FIRMWARE_NAME}  1)
pico_enable_stdio_uart(${FIRMWARE_NAME}  1)

# Add extra outputs (e.g., .bin, .elf, etc.)
pico_add_extra_outputs(${FIRMWARE_NAME} )
