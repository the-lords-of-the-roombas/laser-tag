
add_executable(test-terminal test-terminal.cpp)
target_link_libraries(test-terminal arduino)

avr_hex(test-terminal hex_file test-terminal)
add_custom_target(test-terminal-hex ALL DEPENDS ${hex_file})

add_arduino_executable(base base.cpp)
target_link_libraries(base arduino radio)
