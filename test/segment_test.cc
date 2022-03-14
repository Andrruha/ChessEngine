#include <cstdint>
#include <random>

#include <catch2/catch_all.hpp>

#include "chess_defines.h"

#include <iostream>

bool NaiveImplementation(chess_engine::Segment segment, chess_engine::Coordinates point) {
  if (segment.start == point) {
    return true;
  }
  chess_engine::Coordinates step;
  if (segment.end.file - segment.start.file > 0) {
    step.file = 1;
  } else if (segment.end.file - segment.start.file < 0) {
    step.file = -1;
  } else {
    step.file = 0;
  }
  if (segment.end.rank - segment.start.rank > 0) {
    step.rank = 1;
  } else if (segment.end.rank - segment.start.rank < 0) {
    step.rank = -1;
  } else {
    step.rank = 0;
  }
  chess_engine::Coordinates stop = segment.end;
  stop.file += step.file;
  stop.rank += step.rank;
  for (chess_engine::Coordinates current = segment.start; current != stop; current.file += step.file, current.rank += step.rank) {
    if (current == point) {
      return true;
    }
  }
  return false;
}

chess_engine::Coordinates RandomPoint(std::mt19937& generator) {
  std::uniform_int_distribution<int8_t> random_coordinate(0,7);
  return {random_coordinate(generator), random_coordinate(generator)};
}

chess_engine::Segment RandomSegment(std::mt19937& generator) {
  chess_engine::Segment ret;
  std::uniform_int_distribution<int8_t> random_coordinate(0,7);
  std::uniform_int_distribution<int8_t> random_delta(-1,1);

  ret.start = RandomPoint(generator);
  chess_engine::Coordinates delta = {random_delta(generator), random_delta(generator)};
  int8_t length = random_coordinate(generator);
  ret.end.file = ret.start.file + delta.file*length;
  ret.end.rank = ret.start.rank + delta.rank*length; 
  return ret;
}

TEST_CASE("BelongsToSegment function works", "[defines]") {
  std::mt19937 mt(12345);
  chess_engine::Segment segment = {{2,5},{2,2}};
  chess_engine::Coordinates point = {2,5};
  bool efficient = BelongsToSegment(segment, point);
  bool naive = NaiveImplementation(segment, point);
  REQUIRE(efficient == naive);
  for (int i = 0; i < 10000; ++i) {
    chess_engine::Segment segment = RandomSegment(mt);
    chess_engine::Coordinates point = RandomPoint(mt);
    bool efficient = BelongsToSegment(segment, point);
    bool naive = NaiveImplementation(segment, point);
    REQUIRE(efficient == naive);
  }
}