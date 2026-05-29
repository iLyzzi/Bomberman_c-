cmake_minimum_required(VERSION 4.1)
project(bomberman)

set(CMAKE_CXX_STANDARD 20)


add_executable(bomberman main.cpp
        resources/mapas.cpp
        resources/mapas.h)
target_link_options(bomberman PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
target_link_libraries(bomberman winmm)

