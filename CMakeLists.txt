cmake_minimum_required (VERSION 3.23.0)

project(ChessEngine)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(test)