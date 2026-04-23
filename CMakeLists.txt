cmake_minimum_required(VERSION 3.16)
project(wolf C)

set(CMAKE_C_STANDARD 11)

find_package(SDL2 REQUIRED)

add_executable(wolf src/main.c)

target_include_directories(wolf PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(wolf PRIVATE ${SDL2_LIBRARIES})
