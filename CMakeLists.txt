cmake_minimum_required(VERSION 3.28)
project(TinySDL3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL3 REQUIRED CONFIG)

add_executable(TinyGame "src/main.cpp")
target_link_libraries(TinyGame PRIVATE SDL3::SDL3)
