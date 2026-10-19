cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD adafruit_kb2040)
include(pico_sdk_import.cmake)

project(macropad C CXX ASM)
pico_sdk_init()

add_executable(macropad
    src/main.c
    src/usb_descriptors.c
)

target_include_directories(macropad PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)
target_link_libraries(macropad pico_stdlib pico_unique_id tinyusb_device)

pico_enable_stdio_usb(macropad 0)
pico_enable_stdio_uart(macropad 1)
pico_add_extra_outputs(macropad)