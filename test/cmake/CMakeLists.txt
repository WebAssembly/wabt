cmake_minimum_required(VERSION 3.16)
project(example CXX)

enable_testing()

find_package(wabt REQUIRED)

add_executable(example example.cpp)
target_link_libraries(example PRIVATE wabt::wabt)

add_test(NAME example COMMAND example)
