cmake_minimum_required(VERSION 3.16)
project(wolf C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)

option(DEBUG_NO_AMMO "Start with zero ammo for pistol whip testing" OFF)
option(DEBUG_SHOTGUN "Start with shotgun unlocked for testing" OFF)
set(DEBUG_START_LEVEL "0" CACHE STRING "Skip to this level on startup (0 = disabled)")

file(GLOB_RECURSE SOURCES src/*.c)
add_executable(wolf ${SOURCES})

if(DEBUG_NO_AMMO)
    target_compile_definitions(wolf PRIVATE DEBUG_NO_AMMO)
endif()
if(DEBUG_SHOTGUN)
    target_compile_definitions(wolf PRIVATE DEBUG_SHOTGUN)
endif()
if(NOT DEBUG_START_LEVEL STREQUAL "0")
    target_compile_definitions(wolf PRIVATE DEBUG_START_LEVEL=${DEBUG_START_LEVEL})
endif()

target_include_directories(wolf PRIVATE ${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(wolf PRIVATE ${SDL2_LIBRARIES} SDL2_mixer::SDL2_mixer m)
