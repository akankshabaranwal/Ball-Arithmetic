cmake_minimum_required(VERSION 3.10)
project(ball_arithmetic C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -madx -mbmi2 -mlzcnt")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build)

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(WARNING "Set CMAKE_BUILD_TYPE to \"Release\" for benchmarking.")
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    message(WARNING "Profiling flags are set and profile target added.")
endif()

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
    src/benchmark.c
)
target_link_libraries(benchmark flint gmp arb)

add_executable(unit-test
    ${BALL_ARITH_SRC}
    src/unittest.c
)
target_link_libraries(unit-test flint gmp arb)

add_executable(apbar2-example
    src/apbar2-example.c
)
target_link_libraries(apbar2-example flint gmp arb)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    add_executable(profile-apbar
        ${BALL_ARITH_SRC}
        src/profile_apbar.c
    )
    target_compile_options(profile-apbar PUBLIC "-g" "-pg")
    target_link_libraries(profile-apbar flint gmp arb)
endif()




