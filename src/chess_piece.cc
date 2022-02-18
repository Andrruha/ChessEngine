#include "chess_piece.h"

#include <cassert>
#include <cstdint>

namespace chess_engine {

Piece GetPieceType(Piece piece) {
  return static_cast<Piece>(static_cast<uint8_t>(piece) & 0x7u);
}

Piece GetPieceColor(Piece piece) {
  return static_cast<Piece>(static_cast<uint8_t>(piece) & (0x3u << 3));
}

Piece SetPieceType(Piece piece, Piece type) {
  assert(GetPieceColor(type) == Piece::kNone);  // There is more info, then just a type
  Piece color = GetPieceColor(piece);
  type = GetPieceType(type);
  return static_cast<Piece>(static_cast<uint8_t>(color)|static_cast<uint8_t>(type));
}

Piece SetPieceColor(Piece piece, Piece color) {
  assert(GetPieceType(color) == Piece::kNone);  // There is more info, then just a color
  Piece type = GetPieceType(piece);
  color = GetPieceType(color);
  return static_cast<Piece>(static_cast<uint8_t>(color)|static_cast<uint8_t>(type));
}

} //  namespace chess_engine