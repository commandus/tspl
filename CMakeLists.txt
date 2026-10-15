cmake_minimum_required(VERSION 3.28)
project(tspl)

set(CMAKE_CXX_STANDARD 11)

add_executable(tspl tspl.cpp main.cpp)
target_link_libraries(tspl setupapi.lib)
