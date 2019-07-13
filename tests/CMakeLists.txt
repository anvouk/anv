cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(anv_tests LANGUAGES C)

include_directories("../src")

add_executable(test main.c pch.c)
