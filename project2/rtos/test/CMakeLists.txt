
function(add_rtos_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} rtos)

  avr_hex(${name} hex_file ${name})
  add_custom_target(${name}-hex ALL DEPENDS ${hex_file})
endfunction()

add_rtos_test(test-main test_main.cpp)
add_rtos_test(test-system-create test_system_create.cpp)
add_rtos_test(test-periodic-create test_periodic_create.cpp)
add_rtos_test(test-periodic-schedule test_periodic_schedule.cpp)
add_rtos_test(test-periodic-schedule-fault test_periodic_schedule_fault.cpp)
add_rtos_test(test-periodic-task-timeout test_periodic_task_timeout.cpp)
add_rtos_test(test-periodic-preempt-timeout test_periodic_preempt_timeout.cpp)
add_rtos_test(test-rr-create test_rr_create.cpp)
add_rtos_test(test-rr-interleave test_rr_interleave.cpp)

add_rtos_test(test-time test_time.cpp)

add_rtos_test(test-service-system-to-system test_service_system_to_system.cpp)
add_rtos_test(test-service-periodic-to-system test_service_periodic_to_system.cpp)
add_rtos_test(test-service-rr-to-system test_service_periodic_to_system.cpp)
add_rtos_test(test-service-isr-to-system test_service_isr_to_system.cpp)
