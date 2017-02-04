cmake_minimum_required(VERSION 2.8)
project(ShipEscape)

set(CMAKE_CXX_FLAGS "-O3 -std=c++14 -Wall -Wextra -pedantic")
message(${CMAKE_CXX_COMPILER})
add_subdirectory(viewer)
