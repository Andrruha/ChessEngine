#ifndef CHESS_ENGINE_SRC_CHESS_DEFINES_
#define CHESS_ENGINE_SRC_CHESS_DEFINES_

// Contains some basic chess structs and enums to be used by the engine.
// All code that works with the engine needs most of it.

#include <array>
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

Player Opponent(Player player);
struct Piece {
  PieceType type;
  Player player;
};

bool operator==(Piece first, Piece second);
bool operator!=(Piece first, Piece second);
namespace pieces {
  Piece kNone = {PieceType::kNone, Player::kNone};
  Piece kPawn = {PieceType::kPawn, Player::kNone};
  Piece kRook = {PieceType::kRook, Player::kNone};
  Piece kKnight = {PieceType::kKnight, Player::kNone};
  Piece kBishop = {PieceType::kBishop, Player::kNone};
  Piece kQueen = {PieceType::kQueen, Player::kNone};
  Piece kKing = {PieceType::kKing, Player::kNone};
  Piece kWhite = {PieceType::kNone, Player::kWhite};
  Piece kWhitePawn = {PieceType::kPawn, Player::kWhite};
  Piece kWhiteRook = {PieceType::kRook, Player::kWhite};
  Piece kWhiteKnight = {PieceType::kKnight, Player::kWhite};
  Piece kWhiteBishop = {PieceType::kBishop, Player::kWhite};
  Piece kWhiteQueen = {PieceType::kQueen, Player::kWhite};
  Piece kWhiteKing = {PieceType::kKing, Player::kWhite};
  Piece kBlack = {PieceType::kNone, Player::kBlack};
  Piece kBlackPawn = {PieceType::kPawn, Player::kBlack};
  Piece kBlackRook = {PieceType::kRook, Player::kBlack};
  Piece kBlackKnight = {PieceType::kKnight, Player::kBlack};
  Piece kBlackBishop = {PieceType::kBishop, Player::kBlack};
  Piece kBlackQueen = {PieceType::kQueen, Player::kBlack};
  Piece kBlackKing = {PieceType::kKing, Player::kBlack};
}

std::array<PieceType,4> kPromotionOptions = {
  PieceType::kRook, PieceType::kKnight,
  PieceType::kBishop, PieceType::kQueen
};

enum struct Castle {
  kKingside = 0,
  kQueenside = 1
};

struct Coordinates {
  int8_t file;  // aka "x-coordinate" (0 corresponds to the a-file)
  int8_t rank;  // aka "y-coordinate" (0 corresponds to the first rank)
};

bool operator==(Coordinates first, Coordinates second);
bool operator!=(Coordinates first, Coordinates second);

sturct Segment {
  Coordinates start;
  Coordinates end;
}

bool BelongsToLine(Segment line, Coordinates point);
bool BelongsToSegment(Segment segmant, Coordinates point);

struct Move {
  Coordinates from; 
  Coordinates to;
  Piece piece;  // Piece that moves or piece to promote to.
};

bool operator==(Move first, Move second);
bool operator!=(Move first, Move second);

int8_t DoubleJumpRank(Player player);
int8_t PromotionRank(Player player);
int8_t PawnDirection(Player player);

bool WithinTheBoard(Coordinates square);

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_CHESS_DEFINES_