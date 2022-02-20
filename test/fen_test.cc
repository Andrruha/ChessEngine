#include <vector>

#include <catch2/catch_all.hpp>

#include "chess_defines.h"
#include "fen.h"

TEST_CASE("Fen handles piece names correctly", "[fen]") {
  std::vector<unsigned char> fens{
    'P', 'R', 'N', 'B', 'Q', 'K', 'p', 'r', 'n', 'b', 'q', 'k'
  };
  for (auto fen : fens) {
    chess_engine::Piece piece = chess_engine::FenToPiece(fen);
    unsigned char back_to_fen = chess_engine::PieceToFen(piece);
    REQUIRE(fen == back_to_fen);
  }
}
