cmake_minimum_required (VERSION 2.8.11)
project (C8)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable (c8
	src/main.cc)

target_link_libraries(c8 ${CURSES_LIBRARIES})
