cmake_minimum_required(VERSION 3.17)
project(gameoflife)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
add_executable(gameoflife ${SOURCE_FILES})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED system graphics window network audio)

include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(gameoflife ${SFML_LIBRARIES})
target_link_libraries(gameoflife ${JSONCPP_LIBRARIES})