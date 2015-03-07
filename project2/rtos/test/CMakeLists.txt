
function(add_rtos_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} rtos)

  avr_hex(${name} hex_file ${name})
  add_custom_target(${name}-hex ALL DEPENDS ${hex_file})
endfunction()

add_rtos_test(test-time test_time.cpp)

add_rtos_test(test-periodic-schedule test_periodic_schedule.cpp)

add_rtos_test(test-publish-system test_publish_system.cpp)

add_rtos_test(test-publish-periodic test_publish_periodic.cpp)
