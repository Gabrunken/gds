cmake_minimum_required(VERSION 3.14)
project(GDS C)

add_library(gds INTERFACE)
target_include_directories(gds INTERFACE data_structures)