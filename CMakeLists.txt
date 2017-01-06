cmake_minimum_required(VERSION 3.3)
project(zos)

set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES main.cpp Fat.cpp Fat.h)
add_executable(zos ${SOURCE_FILES})