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
  const Piece kNone = {PieceType::kNone, Player::kNone};
  const Piece kPawn = {PieceType::kPawn, Player::kNone};
  const Piece kRook = {PieceType::kRook, Player::kNone};
  const Piece kKnight = {PieceType::kKnight, Player::kNone};
  const Piece kBishop = {PieceType::kBishop, Player::kNone};
  const Piece kQueen = {PieceType::kQueen, Player::kNone};
  const Piece kKing = {PieceType::kKing, Player::kNone};
  const Piece kWhite = {PieceType::kNone, Player::kWhite};
  const Piece kWhitePawn = {PieceType::kPawn, Player::kWhite};
  const Piece kWhiteRook = {PieceType::kRook, Player::kWhite};
  const Piece kWhiteKnight = {PieceType::kKnight, Player::kWhite};
  const Piece kWhiteBishop = {PieceType::kBishop, Player::kWhite};
  const Piece kWhiteQueen = {PieceType::kQueen, Player::kWhite};
  const Piece kWhiteKing = {PieceType::kKing, Player::kWhite};
  const Piece kBlack = {PieceType::kNone, Player::kBlack};
  const Piece kBlackPawn = {PieceType::kPawn, Player::kBlack};
  const Piece kBlackRook = {PieceType::kRook, Player::kBlack};
  const Piece kBlackKnight = {PieceType::kKnight, Player::kBlack};
  const Piece kBlackBishop = {PieceType::kBishop, Player::kBlack};
  const Piece kBlackQueen = {PieceType::kQueen, Player::kBlack};
  const Piece kBlackKing = {PieceType::kKing, Player::kBlack};
}

const std::array<PieceType,4> kPromotionOptions = {
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
  Coordinates& operator+=(Coordinates other);
  Coordinates operator-();
};

bool operator==(Coordinates first, Coordinates second);
bool operator!=(Coordinates first, Coordinates second);
Coordinates operator+(Coordinates first, Coordinates second);

bool KnightMoveAway(Coordinates first, Coordinates second);
int8_t DistanceSquared(Coordinates first, Coordinates second);
struct Segment {
  Coordinates start;
  Coordinates end;
};

bool BelongsToLine(Segment line, Coordinates point);
bool BelongsToSegment(Segment segment, Coordinates point);

struct Move {
  Coordinates from; 
  Coordinates to;
  Piece piece;  // Piece that moves or piece to promote to.
};

const Move kNullMove = {{-1,-1}, {-1,-1}, pieces::kNone};

bool operator==(Move first, Move second);
bool operator!=(Move first, Move second);

int8_t DoubleJumpRank(Player player);
int8_t PromotionRank(Player player);
int8_t PawnDirection(Player player);

bool WithinTheBoard(Coordinates square);

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_CHESS_DEFINES_