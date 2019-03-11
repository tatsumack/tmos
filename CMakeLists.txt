cmake_minimum_required(VERSION 3.12.3 FATAL_ERROR)

project(TMOS)

add_executable(
    TMOS
    src/bootpack.c
    src/graphic.c
    src/desctbl.c
    src/ascii_fonts.c
)

