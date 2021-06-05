cmake_minimum_required(VERSION 3.10)
project(ball_arithmetic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -madx -mbmi2 -mlzcnt")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        message(WARNING "Set CMAKE_BUILD_TYPE to \"Release\" for benchmarking.")
endif()

include_directories(include)

add_executable(arblib-test
        src/arb-test.c
)

target_link_libraries(arblib-test arb flint)

add_executable(benchmark
        src/benchmark.c)

add_executable(unit-test
        src/unittest.c
)

add_executable(example
        src/example.c
)

target_link_libraries(unit-test flint gmp arb)
target_link_libraries(benchmark flint gmp arb)
target_link_libraries(example flint gmp arb)
