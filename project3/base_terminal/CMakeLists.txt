cmake_minimum_required(VERSION 2.8)

project(robot_tag_game_terminal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5SerialPort REQUIRED)

set(src
  main.cpp
  main_win.cpp
  comm.cpp
  #serial.cpp
)

add_executable(base-terminal ${src})

target_link_libraries(base-terminal Qt5::Widgets Qt5::SerialPort)
