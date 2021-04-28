# v3.13 is current on Debian stable.
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -madx")

project(ball-arithmetic C)

add_executable(example
    apint.c
    apfp.c
    example.c
)
