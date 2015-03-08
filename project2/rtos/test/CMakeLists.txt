
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

add_rtos_test(test-publish-system test_publish_system.cpp)

add_rtos_test(test-publish-periodic test_publish_periodic.cpp)

add_rtos_test(trace-rr trace_rr.cpp)
add_rtos_test(trace-service trace_service.cpp)
