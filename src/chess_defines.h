#ifndef CHESS_ENGINE_SRC_CHESS_DEFINES_
#define CHESS_ENGINE_SRC_CHESS_DEFINES_

// Contains some basic chess structs and enums to be used by the engine.
// All code that works with the engine needs most of it.

#include <cstdint>

namespace chess_engine {

enum struct PieceType : uint8_t {
  kNone   = 0,
  kPawn   = 1,
  kRook   = 2,
  kKnight = 3,
  kBishop = 4,
  kQueen  = 5,
  kKing   = 6
};

enum struct Player {
  kNone  = 0,
  kWhite = 1,
  kBlack = 2
};

struct Piece {
  PieceType type;
  Player player;
}

enum struct Castle {
  kKingside = 0,
  kQueenside = 1
};

struct Coordinates {
  int8_t file;  // aka "x-coordinate" (0 corresponds to the a-file)
  int8_t rank;  // aka "y-coordinate" (0 corresponds to the first rank)
};

struct Move {
  Coordinates from; 
  Coordinates to;
  Piece piece;  // Piece that moves or piece to promote to.
};

int8_t DoubleJumpRank(Player player);
int8_t PromotionRank(Player player);
int8_t PawnDirection(Player player);

bool WithinTheBoard(Coordinates square);

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_CHESS_DEFINES_