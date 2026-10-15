cmake_minimum_required(VERSION 3.28)
project(tspl)

set(CMAKE_CXX_STANDARD 11)

add_executable(tspl tspl.cpp main.cpp)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(EXTRA_LIBS setupapi.lib)
else()
    set(EXTRA_LIBS usb-1.0)
endif()
target_link_libraries(tspl ${EXTRA_LIBS})
