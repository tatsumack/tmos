cmake_minimum_required(VERSION 3.12.3 FATAL_ERROR)

project(TMOS)

file(GLOB sources src/*.c)

add_executable(
    TMOS
    ${sources}
)

