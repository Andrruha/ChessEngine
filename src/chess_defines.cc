#include "chess_defines.h"

#include <cassert>
#include <cstdint>

namespace chess_engine {

Player Opponent(Player player) {
  if (player == Player::kWhite) {
    return Player::kBlack;
  } else if (player == Player::kBlack) {
    return Player::kWhite;
  }
  assert(false);  // invalid player
  return Player::kNone;
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

bool operator!=(Coordinates first, Coordinates second) {
  return !(first == second);
}

bool BelongsToLine(Segment line, Coordinates point) {
  Coordinates big_delta;
  big_delta.file = segment.end.file - segment.start.file;
  big_delta.rank = segment.end.rank - segment.start.rank;
  Coordinates small_delta;
  small_delta.file = point.file - segment.start.file;
  small_delta.rank = point.rank - segment.start.rank;
  return small_delta.file*big_delta.rank == small_delta.rank*big_delta.file;
}

bool BelongsToSegment(Segment segment, Coordinates point) {
  Coordinates big_delta;
  big_delta.file = segment.end.file - segment.start.file;
  big_delta.rank = segment.end.rank - segment.start.rank;
  Coordinates small_delta;
  small_delta.file = point.file - segment.start.file;
  small_delta.rank = point.rank - segment.start.rank;
  Coordinates raminder;
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
  return first.from == second.from && first.to == second.to && first.piece == second.piece;
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
  assert(false);  // Invalid player value
  return -1;
}

int8_t PromotionRank(Player player) {
  if (player == Player::kWhite) {
    return 7;
  } else if (player == Player::kBlack) {
    return 0;
  }
  assert(false);  // Invalid player value
  return -1;
}

int8_t PawnDirection(Player player) {
  if (player == Player::kWhite) {
    return 1;
  } else if (player == Player::kBlack) {
    return -1;
  }
  assert(false);  // Invalid player value
  return 0;
}

bool WithinTheBoard(Coordinates square) {
  return 0 < square.file && square.file < 7 && 
         0 < square.rank && square.rank < 7;  
}

}  // namespace chess_engine