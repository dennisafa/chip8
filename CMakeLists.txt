cmake_minimum_required(VERSION 3.14)
project(chip8)

set(CMAKE_CXX_STANDARD 11)
find_package(SDL2 REQUIRED)
include_directories(chip8 ${SDL2_INCLUDE_DIRS})

add_executable(chip8 main.cpp instruction.h)
target_link_libraries(chip8 ${SDL2_LIBRARIES})