cmake_minimum_required(VERSION 3.10)
project(ball_arithmetic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -madx -mbmi2")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

include_directories(include)

add_executable(arblib-test
        src/arb-test.c
)
target_link_libraries(arblib-test arb flint)

set(BALL_ARITH_SRC
        src/apint.c
        src/apfp.c
        src/apbar.c
)

add_executable(benchmark
        ${BALL_ARITH_SRC}
        src/benchmark.c)

add_executable(unit-test
        ${BALL_ARITH_SRC}
        src/unittest.c
)

target_link_libraries(benchmark flint gmp arb)
target_link_libraries(unit-test flint gmp arb)
