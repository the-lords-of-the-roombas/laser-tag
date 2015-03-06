cmake_minimum_required(VERSION 2.8)

include(../cmake-modules/arduino.cmake)

project(rtos)

add_library(rtos STATIC rtos/os.cpp)
