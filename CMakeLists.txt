cmake_minimum_required(VERSION 3.12.3 FATAL_ERROR)

project(tmos)

file(GLOB sources src/*.c src/app/*.c)

add_executable(
    tmos
    ${sources}
)

