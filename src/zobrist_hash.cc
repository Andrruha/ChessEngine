#include "zobrist_hash.h"

#include <cstdint>
#include <random>

namespace chess_engine {

ZobristHashFunction::ZobristHashFunction(uint64_t seed) {
  std::mt19937_64 generator(seed);
  for (auto &piece_table : white_piece_tables_) {
    for (auto &file : piece_table) {
      for (auto &square : file) {
        square = generator();
      }
    }
  }
  for (auto &piece_table : black_piece_tables_) {
    for (auto &file : piece_table) {
      for (auto &square : file) {
        square = generator();
      }
    }
  }
  for (auto &file : en_pessant_) {
    for (auto &square : file) {
      square = generator();
    }
  }
  white_castles_kingside_ = generator();
  white_castles_queenside_ = generator();
  black_castles_kingside_ = generator();
  white_castles_queenside_ = generator();;
  turn_ = generator();
}

uint64_t ZobristHashFunction::HashPiece(Coordinates square, Piece piece) const {
  if (piece.player == Player::kWhite) {
    return white_piece_tables_[static_cast<int8_t>(piece.type)-1][square.file][square.rank];
  } else if (piece.player == Player::kBlack){
    return black_piece_tables_[static_cast<int8_t>(piece.type)-1][square.file][square.rank];
  } else {
    return 0ull;  // Empty squares
  }
}

uint64_t ZobristHashFunction::HashEnPessant(Coordinates square) const {
  return en_pessant_[square.file][square.rank];
}

uint64_t ZobristHashFunction::HashCastles(Player player, Castle castle) const {
  if (player == Player::kWhite) {
    return castle == Castle::kKingside ? white_castles_kingside_ : white_castles_queenside_;
  } else {
    return castle == Castle::kKingside ? black_castles_kingside_ : black_castles_queenside_;
  }
}

uint64_t ZobristHashFunction::HashTurn() const {
  return turn_;
}

ZobristHash::ZobristHash(const ZobristHashFunction& func):
func_(func)
{};

ZobristHash::ZobristHash(const ZobristHash& other)
:func_(other.func_), hash_(other.hash_)
{}

void ZobristHash::ToggleSquare(Coordinates square, Piece piece) {
  hash_ ^= func_.HashPiece(square, piece);
}

void ZobristHash::ToggleEnPessant(Coordinates square) {
  if (square == Coordinates{-1,-1}) {
    return;
  }
  hash_ ^= func_.HashEnPessant(square);
}

void ZobristHash::ToggleCastlingRights(Player player, Castle castle) {
  hash_ ^= func_.HashCastles(player, castle);
}

void ZobristHash::PassTheTurn() {
  hash_ ^= func_.HashTurn();
}

}  // namespace chess_engine