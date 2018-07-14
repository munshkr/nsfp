cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

find_package(SDL2 REQUIRED)
# FIXME Add find_package for libgme

option(NCURSES "Use ncurses" ON)

if(CURSES)
  message("-- Use ncurses")
  add_definitions(-DCURSES)
  set(CURSES_USE_NCURSES TRUE)
  find_package(Curses REQUIRED)
endif(CURSES)

set(SRC src/main.cc
        src/player.cc)

add_executable(nsfp ${SRC})
target_link_libraries(nsfp LINK_PUBLIC ${CURSES_LIBRARIES} ${SDL2_LIBRARIES} gme)

install (TARGETS nsfp DESTINATION bin)
