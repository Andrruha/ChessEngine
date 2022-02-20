add_library(EngineLibrary chess_defines.cc fen.cc)
target_include_directories(EngineLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Engine main.cc)