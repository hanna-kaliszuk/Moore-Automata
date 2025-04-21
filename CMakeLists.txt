cmake_minimum_required(VERSION 3.29)
project(moore_aut C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(moore_aut
        ma.c
        ma.h
        ma_additional.c
        ma_additional.h)
