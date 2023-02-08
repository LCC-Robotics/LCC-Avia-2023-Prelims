cmake_minimum_required(VERSION 3.0.0)
project(
    "Preliminary Programming Problems" 
    VERSION 0.1.0 
    LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wpedantic -Wextra -fexceptions)

add_executable(Prelim1 Prelim1.cpp)
add_executable(Prelim2 Prelim2.cpp)
add_executable(Prelim3 Prelim3.cpp)
add_executable(Prelim4 Prelim4.cpp)
