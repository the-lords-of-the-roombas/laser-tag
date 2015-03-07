cmake_minimum_required(VERSION 2.8)

include(../cmake-modules/arduino.cmake)

project(rtos)

add_library(rtos STATIC rtos/os.cpp)

add_executable(test test.cpp)
target_link_libraries(test rtos)

avr_hex(test hex_file test)
add_custom_target(test-hex ALL DEPENDS ${hex_file})


