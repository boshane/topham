cmake_minimum_required(VERSION 3.7)
project(topham C)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -Wpedantic -g3 -ggdb)

add_executable(${PROJECT_NAME} main.c
        grid.c
        draw.c
        map.c
        topham.h)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2_ttf")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2_image")

link_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2/build")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2_ttf")
link_directories("${CMAKE_CURRENT_SOURCE_DIR}/sdl/SDL2_image")

target_link_libraries(${PROJECT_NAME} PUBLIC m SDL2 SDL2_ttf SDL2_image)

