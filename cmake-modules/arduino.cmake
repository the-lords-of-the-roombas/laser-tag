cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME Generic)

set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

#set(CSTANDARD "-std=gnu99")
#set(CDEBUG "-g")
set(CWARN "-Wall")
set(CTUNING "-ffunction-sections -fdata-sections")
set(CXXTUNING "-fno-exceptions -ffunction-sections -fdata-sections")
set(COPT "-Os")
set(CMCU "-mmcu=atmega2560 -MMD")
set(CDEFS "-DF_CPU=16000000L -DUSB_VID=null -DUSB_PID=null -DARDUINO=105 -D__PROG_TYPES_COMPAT__")

set(CFLAGS "${CSTANDARD} ${COPT} ${CWARN} ${CTUNING} ${CMCU} ${CDEFS}")
set(CXXFLAGS "${COPT} ${CWARN} ${CXXTUNING} ${CMCU} ${CDEFS}")

set(CMAKE_C_FLAGS "${CFLAGS}")
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")


set(avr_lib_dir /usr/share/arduino/hardware/arduino/cores/arduino/avr-libc)
set(arduino_lib_dir /usr/share/arduino/hardware/arduino/cores/arduino)
set(arduino_mega_lib_dir /usr/share/arduino/hardware/arduino/variants/mega)

set(avr_lib_src_files
  ${avr_lib_dir}/malloc.c
  ${avr_lib_dir}/realloc.c
)

set(arduino_lib_src_files
  #Arduino.h
  #binary.h
  ${arduino_lib_dir}/CDC.cpp
  #Client.h
  ${arduino_lib_dir}/HardwareSerial.cpp
  #HardwareSerial.h
  ${arduino_lib_dir}/HID.cpp
  ${arduino_lib_dir}/IPAddress.cpp
  #IPAddress.h
  ${arduino_lib_dir}/main.cpp
  ${arduino_lib_dir}/new.cpp
  #new.h
  #Platform.h
  #Printable.h
  ${arduino_lib_dir}/Print.cpp
  #Print.h
  #Server.h
  ${arduino_lib_dir}/Stream.cpp
  #Stream.h
  ${arduino_lib_dir}/Tone.cpp
  #Udp.h
  #USBAPI.h
    #${arduino_lib_dir}/USBCore.cpp
  #USBCore.h
  #USBDesc.h
  #WCharacter.h
  ${arduino_lib_dir}/WInterrupts.c
  ${arduino_lib_dir}/wiring_analog.c
  ${arduino_lib_dir}/wiring.c
  ${arduino_lib_dir}/wiring_digital.c
  #wiring_private.h
  ${arduino_lib_dir}/wiring_pulse.c
  ${arduino_lib_dir}/wiring_shift.c
  ${arduino_lib_dir}/WMath.cpp
  ${arduino_lib_dir}/WString.cpp
  #WString.h
)

function(add_arduino_library)
  include_directories(${arduino_lib_dir} ${arduino_mega_lib_dir})
  add_library(arduino STATIC ${avr_lib_src_files} ${arduino_lib_src_files})
endfunction()

function(avr_cpp_object out_var in_file)
  set(out_file "${in_file}.o")
  set(${out_var} ${out_file} PARENT_SCOPE)
  add_custom_command(OUTPUT ${out_file}
    COMMAND avr-g++
    ARGS
      -c -g -Os -Wall
      -fno-exceptions -ffunction-sections -fdata-sections
      -mmcu=atmega2560
      -DF_CPU=16000000L
      -MMD
      -DUSB_VID=null
      -DUSB_PID=null
      -DARDUINO=105
      -D__PROG_TYPES_COMPAT__
      -I/usr/share/arduino/hardware/arduino/cores/arduino
      -I/usr/share/arduino/hardware/arduino/variants/mega
      ${CMAKE_CURRENT_SOURCE_DIR}/${in_file}
      -o ${out_file}
    DEPENDS ${in_file}
  )
endfunction()

function(avr_elf name out_var)
  set(out_file ${name}.elf)
  set(${out_var} ${out_file} PARENT_SCOPE)
  add_custom_command(OUTPUT ${out_file}
    COMMAND avr-g++
    ARGS
      -Os -Wl,--gc-sections,--relax
      -mmcu=atmega2560
      -o ${out_file}
      ${ARGN}
      -lm
    DEPENDS ${ARGN}
  )
endfunction()

function(avr_hex name out_var elf_file)
  set(out_file ${name}.hex)
  set(${out_var} ${out_file} PARENT_SCOPE)
  add_custom_command(OUTPUT ${out_file}
    COMMAND avr-objcopy
    ARGS
      -O ihex
      -R .eeprom
      ${elf_file}
      ${out_file}
    DEPENDS ${elf_file}
  )
endfunction()

function(add_arduino_executable name)

  set(obj_files "")

  foreach(src_file IN LISTS ARGN)
    avr_cpp_object(obj_file ${src_file})
    list(APPEND obj_files ${obj_file})
  endforeach()

  message("Object files: ${obj_files}")

  avr_elf(${name} elf_file ${obj_files})

  avr_hex(${name} hex_file ${elf_file})

  add_custom_target(${name} ALL DEPENDS ${hex_file})

endfunction()
