add_library(EngineLibrary chess_defines.cc fen.cc position.cc count_moves.cc)
target_include_directories(EngineLibrary PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Engine main.cc)
target_link_libraries(Engine EngineLibrary)