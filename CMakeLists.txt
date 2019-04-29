cmake_minimum_required(VERSION 3.13)
project(RTMTreap)

set(GCC_COVERAGE_COMPILE_FLAGS "-mrtm")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
set(CMAKE_CXX_STANDARD 14)

add_executable(RTMTreap main.cpp)