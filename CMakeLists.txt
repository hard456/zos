cmake_minimum_required(VERSION 3.3)
project(zos)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp Fat.cpp Fat.h)
add_executable(zos ${SOURCE_FILES})