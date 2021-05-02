cmake_minimum_required(VERSION 3.10)
project(ball_arithmetic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -madx")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

include_directories(include)

add_executable(arblib-test
        src/arb-test.c
)

target_link_libraries(arblib-test arb flint)
