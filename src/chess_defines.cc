#include "src/chess_defines.h"

#include <cassert>
#include <cstdint>

namespace chess_engine {

Player Opponent(Player player) {
  if (player == Player::kWhite) {
    return Player::kBlack;
  } else if (player == Player::kBlack) {
    return Player::kWhite;
  }
  assert(false);  // Invalid player.
  return Player::kNone;
}

Coordinates& Coordinates::operator+=(Coordinates other) {
  file += other.file;
  rank += other.rank;
  return *this;
}

Coordinates Coordinates::operator-() {
  return {static_cast<int8_t>(-file), static_cast<int8_t>(-rank)};
}

bool operator==(Piece first, Piece second) {
  return first.type == second.type && first.player == second.player;
}

bool operator!=(Piece first, Piece second) {
  return !(first == second);
}

bool operator==(Coordinates first, Coordinates second) {
  return first.file == second.file && first.rank == second.rank;
}

Coordinates operator+(Coordinates first, Coordinates second) {
  return {first.file + second.file, first.rank + second.rank};
}

bool operator!=(Coordinates first, Coordinates second) {
  return !(first == second);
}

bool KnightMoveAway(Coordinates first, Coordinates second) {
  return DistanceSquared(first, second) == 5;
}

int8_t DistanceSquared(Coordinates first, Coordinates second) {
  int8_t file_delta = first.file - second.file;
  int8_t rank_delta = first.rank - second.rank;
  return file_delta*file_delta + rank_delta*rank_delta;
}

bool BelongsToLine(Segment line, Coordinates point) {
  Coordinates big_delta;
  big_delta.file = line.end.file - line.start.file;
  big_delta.rank = line.end.rank - line.start.rank;
  Coordinates small_delta;
  small_delta.file = point.file - line.start.file;
  small_delta.rank = point.rank - line.start.rank;
  
  // dx_1 * dy_2 == dx_2 * dy_1
  return small_delta.file*big_delta.rank == small_delta.rank*big_delta.file;
}

bool BelongsToSegment(Segment segment, Coordinates point) {
  Coordinates big_delta;
  big_delta.file = segment.end.file - segment.start.file;
  big_delta.rank = segment.end.rank - segment.start.rank;
  Coordinates small_delta;
  small_delta.file = point.file - segment.start.file;
  small_delta.rank = point.rank - segment.start.rank;
  Coordinates remainder;
  remainder.file = segment.end.file - point.file;
  remainder.rank = segment.end.rank - point.rank;
  if (small_delta.file*big_delta.rank != small_delta.rank*big_delta.file) {
    return false;
  }
  if (small_delta.file*remainder.file < 0) {
    return false;
  }
  if (small_delta.rank*remainder.rank < 0) {
    return false;
  }
  return true;
}

bool operator==(Move first, Move second) {
  return first.from == second.from &&
    first.to == second.to && first.piece == second.piece;
}

bool operator!=(Move first, Move second) {
  return !(first == second);
}

int8_t DoubleJumpRank(Player player) {
  if (player == Player::kWhite) {
    return 1;
  } else if (player == Player::kBlack) {
    return 6;
  }
  assert(false);  // Invalid player value.
  return -1;
}

int8_t PromotionRank(Player player) {
  if (player == Player::kWhite) {
    return 7;
  } else if (player == Player::kBlack) {
    return 0;
  }
  assert(false);  // Invalid player value.
  return -1;
}

int8_t PawnDirection(Player player) {
  if (player == Player::kWhite) {
    return 1;
  } else if (player == Player::kBlack) {
    return -1;
  }
  assert(false);  // Invalid player value.
  return 0;
}

bool WithinTheBoard(Coordinates square) {
  return 0 <= square.file && square.file < 8 &&
         0 <= square.rank && square.rank < 8;
}

}  // namespace chess_engine
