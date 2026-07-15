cmake_minimum_required(VERSION 3.14)
project(GDS C)

add_library(gdslib INTERFACE)
target_include_directories(gdslib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/data_structures)