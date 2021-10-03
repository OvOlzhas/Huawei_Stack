cmake_minimum_required(VERSION 3.0)
project(Stack)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_EXE main.cpp src/Stack.cpp)

include_directories(includes src)

add_executable(main ${SOURCE_EXE})