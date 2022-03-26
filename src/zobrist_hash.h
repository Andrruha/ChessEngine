#ifndef CHESS_ENGINE_SRC_ZOBRIST_HASH_
#define CHESS_ENGINE_SRC_ZOBRIST_HASH_

#include <cstdint>
#include <array>

#include "chess_defines.h"

namespace chess_engine {

class ZobristHashFunction {
 public:
	ZobristHashFunction(uint64_t seed);
	uint64_t HashPiece(Coordinates square, Piece piece) const;
	uint64_t HashEnPessant(Coordinates square) const;
	uint64_t HashCastles(Player player, Castle castle) const;
	uint64_t HashTurn() const;

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

class ZobristHash{
 public:
  ZobristHash(const ZobristHashFunction& func);
	uint64_t Get() const;
	void ToggleSquare(Coordinates square, Piece piece);
	void ToggleEnPessant(Coordinates square);
	void ToggleCastlingRights(Player player, Castle castle);
	void PassTheTurn();
 private:
 const ZobristHashFunction& func_;
 uint64_t hash_ = 0;
};

}  // namespace chess_engine

#endif  // CHESS_ENGINE_SRC_ZOBRIST_HASH_