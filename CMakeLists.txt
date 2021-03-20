cmake_minimum_required(VERSION 3.10)
project(ball_arithmetic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -madx")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

include_directories(include)
add_executable(ball_arithmetic
        src/arb-test.c
        apint.c
        apfp.c
        apbar.c
        example.c
)

target_link_libraries(ball_arithmetic arb flint)