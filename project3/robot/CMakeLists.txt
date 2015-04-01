
## Sonar test

add_arduino_executable(test-sonar sonar.cpp test-sonar.cpp)
target_link_libraries(test-sonar irobot rtos arduino)


## Drive test

#add_arduino_executable(test-drive test-drive.cpp controller.cpp)
#target_link_libraries(test-drive radio irobot rtos arduino)

## Radio test

add_arduino_executable(test-radio test-radio.cpp)
target_link_libraries(test-radio radio rtos arduino)

## IR Receive / Transmit Test

add_arduino_executable(test-ir-receive test-ir-receive.cpp)
target_link_libraries(test-ir-receive irobot arduino)

add_arduino_executable(test-ir-transmit test-ir-transmit.cpp gun.cpp)
target_link_libraries(test-ir-transmit rtos arduino)

## Robot

add_arduino_executable(robot robot.cpp controller.cpp sequencer.cpp sonar.cpp gun.cpp)
target_link_libraries(robot radio irobot rtos arduino)
