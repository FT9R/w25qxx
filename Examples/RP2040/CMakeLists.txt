cmake_minimum_required(VERSION 3.28)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(rp2040_w25qxx C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(${PROJECT_NAME} ${PROJECT_NAME}.c w25qxx_Interface.c)
add_compile_options(-Wall -Wno-unused-function -Wno-pointer-sign)

# Link w25qxx driver
add_subdirectory(./../../w25qxx ${PROJECT_BINARY_DIR}/w25qxx)
target_include_directories(w25qxx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(w25qxx pico_stdlib hardware_spi)
target_link_libraries(${PROJECT_NAME} w25qxx)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROJECT_NAME})