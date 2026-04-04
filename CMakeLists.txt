cmake_minimum_required(VERSION 3.20)
project(raylib_gaussian_splat_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(raylib CONFIG REQUIRED)

add_executable(raylib_gaussian_splat_demo src/main.cpp)
target_link_libraries(raylib_gaussian_splat_demo PRIVATE raylib)

if (WIN32)
    target_link_libraries(raylib_gaussian_splat_demo PRIVATE opengl32 gdi32 winmm)
endif()
