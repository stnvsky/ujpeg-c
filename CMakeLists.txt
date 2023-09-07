cmake_minimum_required(VERSION 3.22)
project(ujpeg_c C)

set(CMAKE_C_STANDARD 99)

add_executable(ujpeg_c main.c ujpeg.c ujpeg.h)
