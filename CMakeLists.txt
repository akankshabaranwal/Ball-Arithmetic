# v3.13 is current on Debian stable.
cmake_minimum_required(VERSION 3.13)

project(ball-arithmetic C)

add_executable(example
    apint.c
    apfp.c
    example.c
)
