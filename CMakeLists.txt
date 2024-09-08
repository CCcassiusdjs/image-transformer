cmake_minimum_required(VERSION 3.10)
project(T1 C)

set(CMAKE_C_STANDARD 23)
add_executable(T1 main.c)

target_link_libraries(T1 m)  # Adicione esta linha para linkar a biblioteca matemática
