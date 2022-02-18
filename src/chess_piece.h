#ifndef CHESS_ENGINE_SRC_CHESS_PIECES_
#define CHESS_ENGINE_SRC_CHESS_PIECES_

#include <cstdint>

namespace chess_engine {

// Light weight representation of a chess piece to be used in the engine.
// Can be used to specify just piece type or just piece color.
// 3 least significant bits represent piece type.
// Next 2 bits represent piece color.
enum struct Piece : uint8_t {
  kNone        = 000,  // values are given in base 8!
  kPawn        = 001,
  kRook        = 002,
  kKnight      = 003,
  kBishop      = 004,
  kQueen       = 005,
  kKing        = 006,
  kWhite       = 010,
  kWhitePawn   = 011,
  kWhiteRook   = 012,
  kWhiteKnight = 013,
  kWhiteBishop = 014,
  kWhiteQueen  = 015,
  kWhiteKing   = 016,
  kBlack       = 020,
  kBlackPawn   = 021,
  kBlackRook   = 022,
  kBlackKnight = 023,
  kBlackBishop = 024,
  kBlackQueen  = 025,
  kBlackKing   = 026
};

Piece GetPieceType(Piece piece);
Piece GetPieceColor(Piece piece);

// Returns a piece with modified piece type
[[nodiscard]] Piece SetPieceType(Piece piece, Piece type);
// Returns a piece with modified piece color
[[nodiscard]] Piece SetPieceColor(Piece piece, Piece color);

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_CHESS_PIECES_