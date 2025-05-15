cmake_minimum_required(VERSION 3.26)
project(main C CXX)

include_directories(inc/)

find_package(SDL3 REQUIRED)

add_executable(main)

target_sources(main
PRIVATE
    src/main.c
    src/input.c
    src/objects.c
    src/animation.c
    src/render/render_utils.c
    src/render/renderer.c
    src/debug/rdebug.c
    src/debug/memtrack.c
)

target_link_libraries(main SDL3::SDL3)