cmake_minimum_required(VERSION 2.8)

include(../cmake-modules/arduino_head.cmake)

project(uvic_rtsys_project3)

include(../cmake-modules/arduino_tail.cmake)

## Compile arduino lib

set(avr_lib_src_files
  #${avr_lib_dir}/malloc.c
  #${avr_lib_dir}/realloc.c
)

set(arduino_lib_src_files
  #${arduino_lib_dir}/CDC.cpp
  ${arduino_lib_dir}/HardwareSerial.cpp
  #${arduino_lib_dir}/HID.cpp
  #${arduino_lib_dir}/IPAddress.cpp
  #${arduino_lib_dir}/main.cpp
  ${arduino_lib_dir}/new.cpp
  ${arduino_lib_dir}/Print.cpp
  ${arduino_lib_dir}/Stream.cpp
  #${arduino_lib_dir}/Tone.cpp
  #${arduino_lib_dir}/USBCore.cpp
  #${arduino_lib_dir}/WInterrupts.c
  ${arduino_lib_dir}/wiring_analog.c
  ${arduino_lib_dir}/wiring.c
  ${arduino_lib_dir}/wiring_digital.c
  ${arduino_lib_dir}/wiring_pulse.c
  ${arduino_lib_dir}/wiring_shift.c
  #${arduino_lib_dir}/WMath.cpp
  ${arduino_lib_dir}/WString.cpp
)

include_directories(${arduino_lib_dir} ${arduino_mega_lib_dir})

add_library(arduino STATIC ${avr_lib_src_files} ${arduino_lib_src_files})

## Compile RTOS

add_subdirectory(rtos)

## Compile app

set(src
  test.cpp
  gun.cpp
  irobot/irobot.cpp
  radio/radio.c
  radio/spi.c
)

add_executable(test ${src})
target_link_libraries(test rtos arduino)

set_property(TARGET test PROPERTY OUTPUT_NAME test.elf)

## Make HEX

avr_hex(test test_hex_file test.elf)
add_custom_target(test-hex ALL DEPENDS ${test_hex_file})
