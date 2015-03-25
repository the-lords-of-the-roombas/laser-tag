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

add_subdirectory(rtos)
add_subdirectory(irobot)
add_subdirectory(radio)
add_subdirectory(robot)
add_subdirectory(base)
