
function(add_rtos_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} rtos)

  avr_hex(${name} hex_file ${name})
  add_custom_target(${name}-hex ALL DEPENDS ${hex_file})
endfunction()

add_executable(test test.cpp)
target_link_libraries(test rtos)

avr_hex(test hex_file test)
add_custom_target(test-hex ALL DEPENDS ${hex_file})


add_rtos_test(test-publish-system test_publish_system.cpp)

add_rtos_test(test-publish-periodic test_publish_periodic.cpp)
