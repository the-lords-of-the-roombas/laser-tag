add_definitions("-DINIT_ARDUINO_LIB")
#add_definitions("-DTRACE_KERNEL_MODE")
#add_definitions("-DTRACE_KERNEL_MODE_ON_TICK")
#add_definitions("-DTRACE_TASKS")
#add_definitions("-DTRACE_REQUESTS")

add_library(rtos STATIC os.cpp)
