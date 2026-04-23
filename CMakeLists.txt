# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(PICO_SDK_VERSION "2.2.0" CACHE STRING "Pico SDK version for tool paths")
set(sdkVersion ${PICO_SDK_VERSION})
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion ${PICO_SDK_VERSION})
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

# Set path to PICO SDK
#include("$ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake")


# Minimum CMake version
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set the board to Pico W
#set(PICO_BOARD pico_w CACHE STRING "Board type")

# Select correct board. Uncomment correct

# Select target board here.
#set(MY_PICO_BOARD pico)
#set(MY_PICO_BOARD pico_w)
#set(MY_PICO_BOARD pico2)
set(MY_PICO_BOARD pico2_w)


set(PICO_BOARD ${MY_PICO_BOARD} CACHE STRING "Board type")

# OpenOCD target for uploads (e.g. rp2040 or rp2350)
set(PICO_UPLOAD_TARGET "rp2350" CACHE STRING "OpenOCD target file (without .cfg)")
#set(PICO_UPLOAD_TARGET "rp2040" CACHE STRING "OpenOCD target file (without .cfg)")

# J-Link core selection
# RP2040: core 0 -> RP2040_M0_0, core 1 -> RP2040_M0_1
# RP2350: core 0 -> RP2350_M33_0, core 1 -> RP2350_M33_1
set(PICO_JLINK_CORE "0" CACHE STRING "J-Link core index: 0 or 1")

# Remote upload/debug defaults
set(PICO_REMOTE_HOST "mypi5.local" CACHE STRING "Remote host for OpenOCD/SSH tasks")
set(PICO_REMOTE_USER "tanel" CACHE STRING "Remote user for OpenOCD/SSH tasks")
set(PICO_REMOTE_PATH "/home/tanel/picotemp" CACHE STRING "Remote path for firmware uploads")
set(PICO_FIRMWARE_NAME "pico_ble_template_C" CACHE STRING "Firmware name (without extension)" FORCE)


# Include the Pico SDK
include(pico_sdk_import.cmake)

project(piko_sdk_template_C C CXX ASM)

# Location of executable outputs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# UF2 output directory (used by pico_add_extra_outputs via ARCHIVE_OUTPUT_DIRECTORY)
set(PICO_UF2_OUTPUT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "UF2 output directory")


# Initialize the Pico SDK
pico_sdk_init()

# Define the executable (this will be done in src/CMakeLists.txt)
# Do not add target_link_libraries or pico_add_extra_outputs here

# Add subdirectories for libraries and sources
add_subdirectory(lib)
add_subdirectory(src)

# Specify the serial number of your CMSIS-DAP device (replace with actual serial number)
# First probe serial number
#set(CMSIS_DAP_SERIAL_NUMBER "E6616407E311472F")
set(CMSIS_DAP_SERIAL_NUMBER "E6616407E3245729") 
# Second probe serial number
#set(CMSIS_DAP_SERIAL_NUMBER "E6616407E3245729")

# Generate a shell-friendly file that exports the serial, board, and task settings
file(WRITE "${CMAKE_BINARY_DIR}/project_configuration.sh"
"export CMSIS_DAP_SERIAL_NUMBER=${CMSIS_DAP_SERIAL_NUMBER}\n"
"export PICO_BOARD=${PICO_BOARD}\n"
"export PICO_UPLOAD_TARGET=${PICO_UPLOAD_TARGET}\n"
"export PICO_JLINK_CORE=${PICO_JLINK_CORE}\n"
"export PICO_REMOTE_HOST=${PICO_REMOTE_HOST}\n"
"export PICO_REMOTE_USER=${PICO_REMOTE_USER}\n"
"export PICO_REMOTE_PATH=${PICO_REMOTE_PATH}\n"
"export PICO_FIRMWARE_NAME=${PICO_FIRMWARE_NAME}\n"
"export PICO_SDK_VERSION=${PICO_SDK_VERSION}\n")

file(WRITE "${CMAKE_BINARY_DIR}/project_configuration.env"
"CMSIS_DAP_SERIAL_NUMBER=${CMSIS_DAP_SERIAL_NUMBER}\n"
"PICO_BOARD=${PICO_BOARD}\n"
"PICO_UPLOAD_TARGET=${PICO_UPLOAD_TARGET}\n"
"PICO_JLINK_CORE=${PICO_JLINK_CORE}\n"
"PICO_REMOTE_HOST=${PICO_REMOTE_HOST}\n"
"PICO_REMOTE_USER=${PICO_REMOTE_USER}\n"
"PICO_REMOTE_PATH=${PICO_REMOTE_PATH}\n"
"PICO_FIRMWARE_NAME=${PICO_FIRMWARE_NAME}\n"
"PICO_SDK_VERSION=${PICO_SDK_VERSION}\n")

add_custom_target(upload
    COMMAND ${CMAKE_COMMAND} -E echo "Uploading firmware to CMSIS-DAP device with serial: ${CMSIS_DAP_SERIAL_NUMBER}"
    COMMAND picotool upload ${CMAKE_PROJECT_NAME}.uf2 --serial ${CMSIS_DAP_SERIAL_NUMBER}
    COMMENT "Uploading firmware to device"
    VERBATIM
)
