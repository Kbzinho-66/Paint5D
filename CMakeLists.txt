cmake_minimum_required(VERSION 3.0.0)
set (CMAKE_CXX_STANDARD 11)
project(paint VERSION 0.1.0)

add_executable(paint src/main.cpp)

target_link_libraries(paint SDL2)
