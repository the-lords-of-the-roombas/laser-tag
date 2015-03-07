
add_executable(test test.cpp)
target_link_libraries(test rtos)

avr_hex(test hex_file test)
add_custom_target(test-hex ALL DEPENDS ${hex_file})

