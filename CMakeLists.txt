cmake_minimum_required(VERSION 3.16)
project(wolf C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)

file(GLOB_RECURSE SOURCES src/*.c)
add_executable(wolf ${SOURCES})

target_include_directories(wolf PRIVATE ${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(wolf PRIVATE ${SDL2_LIBRARIES} m)
