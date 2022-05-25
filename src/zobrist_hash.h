#ifndef SRC_ZOBRIST_HASH_H_
#define SRC_ZOBRIST_HASH_H_

#include <cstdint>
#include <array>

#include "src/chess_defines.h"
#include "src/position.h"

namespace chess_engine {

// Calculate alculating Zobrist hash, for individual pieces/squares,
// position features, or the entire position. 
class ZobristHashFunction {
 public:
  explicit ZobristHashFunction(uint64_t seed);
  uint64_t HashPiece(Coordinates square, Piece piece) const;
  uint64_t HashEnPessant(Coordinates square) const;
  uint64_t HashCastles(Player player, Castle castle) const;
  uint64_t HashTurn() const;

  // Slow way to hash a position, hash incerementally, when possible
  uint64_t SlowHash(const Position& position) const;
 private:
  std::array<std::array<std::array<uint64_t, 8>, 8>, 6> white_piece_tables_;
  std::array<std::array<std::array<uint64_t, 8>, 8>, 6> black_piece_tables_;
  std::array<std::array<uint64_t, 8>, 8> en_pessant_;
  uint64_t white_castles_kingside_;
  uint64_t white_castles_queenside_;
  uint64_t black_castles_kingside_;
  uint64_t black_castles_queenside_;
  uint64_t turn_;
};

// Represents zobrist hash of a position, and allows to incrementtally 
// calculate it.
class ZobristHash{
 public:
  explicit ZobristHash(const ZobristHashFunction& func);
  ZobristHash(const Position& Position, const ZobristHashFunction& func);
  ZobristHash(const ZobristHash& other);

  uint64_t Get() const;
  operator uint64_t() const;
  void ToggleSquare(Coordinates square, Piece piece);
  void ToggleEnPessant(Coordinates square);
  void ToggleCastlingRights(Player player, Castle castle);
  void PassTheTurn();

  void RecalculateForPosition(const Position& position);

  const ZobristHashFunction& GetHashFunction() const;
 private:
  const ZobristHashFunction& func_;
  uint64_t hash_ = 0;
};

}  // namespace chess_engine

#endif  // SRC_ZOBRIST_HASH_H_
