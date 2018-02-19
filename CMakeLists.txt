cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

find_package(SDL2)
# FIXME Add find_package for libgme

set(SRC src/main.cc
        src/player.cc)

add_executable(nsfp ${SRC})
target_link_libraries(nsfp LINK_PUBLIC SDL2 gme)
