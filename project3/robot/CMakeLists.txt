
## Sonar test

add_arduino_executable(test-sonar sonar.cpp test-sonar.cpp)
target_link_libraries(test-sonar irobot rtos arduino)


## Drive test

add_arduino_executable(test-drive test-drive.cpp controller.cpp)
target_link_libraries(test-drive radio irobot rtos arduino)

## Radio test

add_arduino_executable(test-radio test-radio.cpp)
target_link_libraries(test-radio radio rtos arduino)
