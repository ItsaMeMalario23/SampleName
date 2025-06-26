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
    src/worldsim.c
    src/render/render_utils.c
    src/render/render.c
    src/render/camera.c
    src/levels/lvl_bird.c
    src/levels/lvl_mario.c
    src/levels/lvl_odyssey.c
    src/levels/lvl_air.c
    src/debug/rdebug.c
    src/debug/memtrack.c
)

target_link_libraries(main SDL3::SDL3)