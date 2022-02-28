#include "chess_defines.h"

#include <cassert>
#include <cstdint>

namespace chess_engine {

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