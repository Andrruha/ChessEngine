find_package(Catch2 3 REQUIRED)

add_executable(
  Test
  fen_test.cc
  segment_test.cc
  position_test.cc
  tricky_positions_test.cc
  hash_count_test.cc
)

target_link_libraries(Test Catch2::Catch2WithMain EngineLibrary)