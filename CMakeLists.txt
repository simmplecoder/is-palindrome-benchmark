cmake_minimum_required(VERSION 3.21)
project(is_pal)

set(CMAKE_CXX_STANDARD 17)

add_executable(benchmark main.cpp)
add_executable(array array.cpp)
add_executable(bitset bitset.cpp)
