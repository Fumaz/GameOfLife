cmake_minimum_required(VERSION 3.17)
project(gameoflife)

set(CMAKE_CXX_STANDARD 14)
set(SOURCE_FILES main.cpp)

find_package(Qt5Widgets REQUIRED)

add_executable(gameoflife ${SOURCE_FILES})

target_link_libraries(gameoflife Qt5::Widgets)