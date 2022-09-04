cmake_minimum_required(VERSION 3.0.0)
project(Paint VERSION 0.1.0)

add_executable(Paint src/main.cpp)

target_link_libraries(Paint SDL2)
