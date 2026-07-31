cmake_minimum_required(VERSION 3.25)
project(Os1_shell)

set(CMAKE_CXX_STANDARD 11)

add_executable(Os1_shell signals.h signals.cpp smash.cpp Commands.h Commands.cpp)
