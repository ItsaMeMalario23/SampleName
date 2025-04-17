cmake_minimum_required(VERSION 3.26)
project(main C CXX)

include_directories(inc/)

find_package(SDL3 REQUIRED)

add_executable(main)

target_sources(main
PRIVATE
    src/main.c
    src/com.c
    src/render.c
    src/basicvertex.c
    src/debug/rdebug.c
    src/debug/memtrack.c
)

target_link_libraries(main SDL3::SDL3)