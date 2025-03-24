cmake_minimum_required(VERSION 3.16.3)
project(Jump)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)


add_executable(Jump "Jump.c")
link_directories("./SDL2.dll")
target_link_libraries(Jump SDL2main)
target_link_libraries(Jump SDL2)
target_link_libraries(Jump SDL2_image)
target_link_libraries(Jump SDL2_ttf)