cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "-O3 -Wall")

find_package(SDL2)
# FIXME Add find_package for libgme

add_executable(nsfp src/main.c)
target_link_libraries(nsfp LINK_PUBLIC SDL2 gme)
