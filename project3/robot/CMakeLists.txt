
set(src
  test.cpp
  gun.cpp
)

add_executable(test ${src})
target_link_libraries(test irobot radio rtos arduino)

set_property(TARGET test PROPERTY OUTPUT_NAME test.elf)

## Make HEX

avr_hex(test test_hex_file test.elf)
add_custom_target(test-hex ALL DEPENDS ${test_hex_file})
