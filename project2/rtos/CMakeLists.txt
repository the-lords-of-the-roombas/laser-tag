add_definitions("-DDEBUG_KERNEL_MODE")

add_library(rtos STATIC os.cpp)

add_subdirectory(test)
